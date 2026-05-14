#include <filesystem>
#include <gtest/gtest.h>
#include <runir/datasets/config.hpp>
#include <runir/datasets/state_graph.hpp>
#include <runir/kr/gp/dl/policy_factory.hpp>
#include <runir/kr/gp/policy_executor.hpp>
#include <runir/kr/gp/repository.hpp>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/planning.hpp>
#include <vector>

namespace
{

std::filesystem::path runir_root() { return std::filesystem::path(RUNIR_ROOT_DIR); }

std::filesystem::path benchmark_prefix() { return runir_root() / "data" / "planning-benchmarks"; }

}  // namespace

TEST(RunirTests, FranceEtAlAaai2021PolicyFactoriesExecuteOnExampleTasks)
{
    namespace gp = runir::kr::gp;
    namespace gp_dl = runir::kr::gp::dl;
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    struct Case
    {
        std::filesystem::path domain;
        std::filesystem::path task;
        gp_dl::PolicySpecification specification;
    };

    const auto cases = std::vector<Case> {
        { benchmark_prefix() / "tests" / "classical" / "gripper" / "domain.pddl",
          benchmark_prefix() / "tests" / "classical" / "gripper" / "test-1.pddl",
          gp_dl::PolicySpecification::GRIPPER_FRANCE_ET_AL_AAAI2021 },
        { benchmark_prefix() / "tests" / "classical" / "blocks_3" / "domain.pddl",
          benchmark_prefix() / "tests" / "classical" / "blocks_3" / "test-1.pddl",
          gp_dl::PolicySpecification::BLOCKS3OPS_FRANCE_ET_AL_AAAI2021 },
        { benchmark_prefix() / "tests" / "classical" / "spanner" / "domain.pddl",
          benchmark_prefix() / "tests" / "classical" / "spanner" / "test-1.pddl",
          gp_dl::PolicySpecification::SPANNER_FRANCE_ET_AL_AAAI2021 },
        { benchmark_prefix() / "tests" / "classical" / "delivery" / "domain.pddl",
          benchmark_prefix() / "tests" / "classical" / "delivery" / "test-1.pddl",
          gp_dl::PolicySpecification::DELIVERY_FRANCE_ET_AL_AAAI2021 },
    };

    auto dl_repository_factory = runir::kr::dl::ConstructorRepositoryFactory();
    auto repository_factory = gp::RepositoryFactory();

    for (const auto& test_case : cases)
    {
        auto parser = fp::Parser(test_case.domain);
        const auto planning_task = parser.parse_task(test_case.task);
        auto execution_context = tyr::ExecutionContext::create(1);
        auto lifted_task = p::Task<p::LiftedTag>(planning_task);
        auto task = lifted_task.instantiate_ground_task(*execution_context).task;
        auto context = runir::datasets::TaskSearchContext<p::GroundTag>(task, execution_context);
        const auto state_graph = runir::datasets::generate_state_graph(context);
        const auto annotated_state_graph =
            runir::datasets::annotate_state_graph(context, *state_graph, runir::datasets::StateGraphCostMode::UNIT_COST);

        auto dl_repository = dl_repository_factory.create_shared(task->get_repository());
        auto repository = repository_factory.create(dl_repository);
        const auto policy = gp_dl::PolicyFactory::create(test_case.specification, task->get_domain().get_domain(), repository);
        const auto result = gp::execute_policy(*annotated_state_graph, policy);

        EXPECT_TRUE(result.is_successful()) << test_case.domain;
        EXPECT_TRUE(result.deadend_transitions.empty()) << test_case.domain;
        EXPECT_TRUE(result.open_states.empty()) << test_case.domain;
        EXPECT_TRUE(result.cycle.empty()) << test_case.domain;
        EXPECT_GT(result.graph.get_num_vertices(), 0) << test_case.domain;
    }
}
