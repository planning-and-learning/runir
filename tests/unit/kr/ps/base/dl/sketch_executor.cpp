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

namespace runir::tests
{

namespace
{

std::filesystem::path runir_root() { return std::filesystem::path(RUNIR_ROOT_DIR); }

std::filesystem::path benchmark_prefix() { return runir_root() / "data" / "planning-benchmarks"; }

}  // namespace

TEST(RunirTests, FranceEtAlAaai2021SketchFactoriesExecuteOnExampleTasks)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    struct Case
    {
        std::filesystem::path domain;
        std::filesystem::path task;
        kr::ps::base::dl::SketchSpecification specification;
    };

    const auto cases = std::vector<Case> {
        { benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl",
          benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl",
          kr::ps::base::dl::SketchSpecification::GRIPPER_FRANCE_ET_AL_AAAI2021 },
        { benchmark_prefix() / "tests" / "classical" / "blocks_3" / "domain.pddl",
          benchmark_prefix() / "tests" / "classical" / "blocks_3" / "test-1.pddl",
          kr::ps::base::dl::SketchSpecification::BLOCKS3OPS_FRANCE_ET_AL_AAAI2021 },
        { benchmark_prefix() / "tests" / "classical" / "spanner" / "domain.pddl",
          benchmark_prefix() / "tests" / "classical" / "spanner" / "test-1.pddl",
          kr::ps::base::dl::SketchSpecification::SPANNER_FRANCE_ET_AL_AAAI2021 },
        { benchmark_prefix() / "tests" / "classical" / "delivery" / "domain.pddl",
          benchmark_prefix() / "tests" / "classical" / "delivery" / "test-1.pddl",
          kr::ps::base::dl::SketchSpecification::DELIVERY_FRANCE_ET_AL_AAAI2021 },
    };

    auto dl_repository_factory = kr::dl::base::ConstructorRepositoryFactory();
    auto repository_factory = kr::ps::base::RepositoryFactory();

    for (const auto& test_case : cases)
    {
        auto parser = fp::Parser(test_case.domain);
        const auto planning_task = parser.parse_task(test_case.task);
        auto execution_context = tyr::ExecutionContext::create(1);
        auto lifted_task = p::Task<p::LiftedTag>(planning_task);
        auto task = lifted_task.instantiate_ground_task(*execution_context).task;
        auto context = datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
        auto dl_repository = dl_repository_factory.create_shared(task->get_repository());
        auto repository = repository_factory.create(dl_repository);
        const auto sketch = kr::ps::base::dl::SketchFactory::create(test_case.specification, task->get_domain().get_domain(), *repository);
        const auto result = kr::ps::base::prove_solution(context, sketch);

        EXPECT_TRUE(result.is_successful()) << test_case.domain;
        EXPECT_TRUE(result.deadend_transitions.empty()) << test_case.domain;
        EXPECT_TRUE(result.open_states.empty()) << test_case.domain;
        EXPECT_TRUE(result.cycle.empty()) << test_case.domain;
        EXPECT_GT(result.graph->get_num_vertices(), 0) << test_case.domain;

        const auto search_result = kr::ps::base::find_solution(context, sketch);
        EXPECT_EQ(search_result.status, p::SearchStatus::SOLVED) << test_case.domain;
        EXPECT_TRUE(search_result.plan) << test_case.domain;
    }
}

}  // namespace runir::tests
