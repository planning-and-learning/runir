#include <filesystem>
#include <gtest/gtest.h>
#include <runir/datasets/config.hpp>
#include <runir/datasets/state_graph.hpp>
#include <runir/kr/ps/base/dl/sketch_factory.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/sketch_executor.hpp>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/planning.hpp>
#include <vector>
#include <yggdrasil/execution/onetbb.hpp>

namespace runir::tests
{

namespace
{

std::filesystem::path runir_root() { return std::filesystem::path(RUNIR_ROOT_DIR); }

std::filesystem::path benchmark_prefix() { return runir_root() / "data" / "benchmarks"; }

}  // namespace

TEST(RunirTests, FranceEtAlAaai2021SketchFactoriesExecuteOnExampleTasks)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    EXPECT_THROW(kr::TaskContext<p::GroundTag>::create(datasets::TaskSearchContextPtr<p::GroundTag> {}), std::invalid_argument);

    struct Case
    {
        std::filesystem::path domain;
        std::filesystem::path task;
        kr::ps::base::dl::SketchSpecification specification;
    };

    const auto cases = std::vector<Case> {
        { benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl",
          benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl",
          kr::ps::base::dl::SketchSpecification::GRIPPER_FRANCE_ET_AL_AAAI2021 },
        { benchmark_prefix() / "classical" / "tests" / "blocks_3" / "domain.pddl",
          benchmark_prefix() / "classical" / "tests" / "blocks_3" / "test-1.pddl",
          kr::ps::base::dl::SketchSpecification::BLOCKS3OPS_FRANCE_ET_AL_AAAI2021 },
        { benchmark_prefix() / "classical" / "tests" / "spanner" / "domain.pddl",
          benchmark_prefix() / "classical" / "tests" / "spanner" / "test-1.pddl",
          kr::ps::base::dl::SketchSpecification::SPANNER_FRANCE_ET_AL_AAAI2021 },
        { benchmark_prefix() / "classical" / "tests" / "delivery" / "domain.pddl",
          benchmark_prefix() / "classical" / "tests" / "delivery" / "test-1.pddl",
          kr::ps::base::dl::SketchSpecification::DELIVERY_FRANCE_ET_AL_AAAI2021 },
    };

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>();
    auto repository_factory = kr::ps::base::RepositoryFactory();

    for (const auto& test_case : cases)
    {
        auto parser = fp::Parser(test_case.domain);
        const auto planning_task = parser.parse_task(test_case.task);
        auto execution_context = ygg::ExecutionContext::create(1);
        auto lifted_task = p::Task<p::LiftedTag>(planning_task);
        auto task = lifted_task.instantiate_ground_task(*execution_context).task;
        auto context = datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
        auto task_context = kr::TaskContext<p::GroundTag>::create(context);
        auto dl_repository = dl_repository_factory.create(task->get_repository());
        auto repository = repository_factory.create(dl_repository);
        const auto sketch = kr::ps::base::dl::SketchFactory::create(test_case.specification, task->get_domain().get_domain(), *repository);
        const auto* dl_builder = &task_context->dl_builder;
        const auto* dl_denotation_repository = task_context->dl_denotation_repository.get();
        const auto result = kr::ps::base::prove_solution(task_context, sketch);

        EXPECT_TRUE(result.is_successful()) << test_case.domain;
        EXPECT_TRUE(result.deadend_transitions.empty()) << test_case.domain;
        EXPECT_TRUE(result.open_states.empty()) << test_case.domain;
        EXPECT_TRUE(result.cycle.empty()) << test_case.domain;
        EXPECT_GT(result.graph->get_num_vertices(), 0) << test_case.domain;

        const auto fragment = kr::ps::base::find_solution(task_context, sketch);
        EXPECT_TRUE(fragment.is_successful()) << test_case.domain;
        ASSERT_TRUE(fragment.graph) << test_case.domain;
        EXPECT_GT(fragment.graph->get_num_vertices(), 0) << test_case.domain;
        EXPECT_EQ(fragment.graph->get_num_edges() + 1, fragment.graph->get_num_vertices()) << test_case.domain;
        EXPECT_LE(fragment.graph->get_num_vertices(), result.graph->get_num_vertices()) << test_case.domain;

        auto expander = kr::ps::base::SuccessorExpander<p::GroundTag>(*task_context, sketch);
        auto evaluation_context = expander.context_at(context->successor_generator->get_initial_node().get_state());
        EXPECT_EQ(task_context->search_context.get(), context.get());
        EXPECT_EQ(task_context->search_context, context);
        EXPECT_EQ(dl_builder, &task_context->dl_builder);
        EXPECT_EQ(dl_builder, &evaluation_context.get_dl_builder());
        EXPECT_EQ(dl_denotation_repository, task_context->dl_denotation_repository.get());
        EXPECT_EQ(dl_denotation_repository, &evaluation_context.get_dl_denotation_repository());
    }
}

}  // namespace runir::tests
