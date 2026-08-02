#include "fixtures.hpp"
#include "planning_fixtures.hpp"

#include <algorithm>
#include <filesystem>
#include <gtest/gtest.h>
#include <runir/datasets/state_graph.hpp>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/dl/sketch_factory.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/sketch_executor.hpp>
#include <runir/kr/ps/base/successor_expander.hpp>
#include <vector>

namespace runir::tests
{

TEST(RunirTests, SketchProofStatusToString)
{
    using kr::ps::base::SketchProofStatus;
    EXPECT_EQ(kr::ps::base::to_string(SketchProofStatus::SUCCESS), "success");
    EXPECT_EQ(kr::ps::base::to_string(SketchProofStatus::FAILURE), "failure");
    EXPECT_EQ(kr::ps::base::to_string(SketchProofStatus::OUT_OF_TIME), "out_of_time");
    EXPECT_EQ(kr::ps::base::to_string(SketchProofStatus::OUT_OF_STATES), "out_of_states");
    EXPECT_THROW((void) kr::ps::base::to_string(static_cast<SketchProofStatus>(255)), std::invalid_argument);
}

TEST(RunirTests, FranceEtAlAaai2021SketchFactoriesExecuteOnExampleTasks)
{
    namespace p = tyr::planning;

    EXPECT_THROW(kr::TaskContext<p::GroundTag>::create(datasets::TaskSearchContextPtr<p::GroundTag> {}), std::invalid_argument);

    struct Case
    {
        std::filesystem::path domain;
        std::filesystem::path task;
        kr::ps::base::dl::SketchSpecification specification;
    };

    const auto cases = std::vector<Case> {
        { benchmark_path("classical/tests/gripper/domain.pddl"),
          benchmark_path("classical/tests/gripper/test-1.pddl"),
          kr::ps::base::dl::SketchSpecification::GRIPPER_FRANCE_ET_AL_AAAI2021 },
        { benchmark_path("classical/tests/blocks_3/domain.pddl"),
          benchmark_path("classical/tests/blocks_3/test-1.pddl"),
          kr::ps::base::dl::SketchSpecification::BLOCKS3OPS_FRANCE_ET_AL_AAAI2021 },
        { benchmark_path("classical/tests/spanner/domain.pddl"),
          benchmark_path("classical/tests/spanner/test-1.pddl"),
          kr::ps::base::dl::SketchSpecification::SPANNER_FRANCE_ET_AL_AAAI2021 },
        { benchmark_path("classical/tests/delivery/domain.pddl"),
          benchmark_path("classical/tests/delivery/test-1.pddl"),
          kr::ps::base::dl::SketchSpecification::DELIVERY_FRANCE_ET_AL_AAAI2021 },
    };

    for (const auto& test_case : cases)
    {
        auto context = make_ground_context(test_case.domain, test_case.task);
        auto task = context->task;
        auto task_context = kr::TaskContext<p::GroundTag>::create(context);
        auto dl_repository = task_context->base_dl_repository;
        auto repository = task_context->base_repository;
        const auto sketch = kr::ps::base::dl::SketchFactory::create(test_case.specification, task->get_domain().get_domain(), *repository);
        const auto* dl_builder = &task_context->dl_builder;
        const auto* dl_denotation_repository = task_context->dl_denotation_repository.get();
        auto proof_options = kr::ps::base::SketchSearchOptions<p::GroundTag> {};
        proof_options.universal = true;
        const auto result = kr::ps::base::find_solution(task_context, sketch, proof_options);

        EXPECT_TRUE(result.is_successful()) << test_case.domain;
        EXPECT_TRUE(result.deadend_states.empty()) << test_case.domain;
        EXPECT_TRUE(result.open_states.empty()) << test_case.domain;
        EXPECT_TRUE(result.cycle.empty()) << test_case.domain;
        EXPECT_GT(result.graph->get_num_vertices(), 0) << test_case.domain;

        auto search_options = kr::ps::base::SketchSearchOptions<p::GroundTag> {};
        const auto fragment = kr::ps::base::find_solution(task_context, sketch, search_options);
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

TEST(RunirTests, BaseFindSolutionUsesOnlyImmediateOutcomesAndUniversalUsesAll)
{
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->base_dl_repository;
    auto repository = task_context->base_repository;
    const auto sketch = kr::ps::base::dl::parse_sketch(read_fixture("kr/ps/base/executor/any_transition.sketch"), task->get_domain().get_domain(), *repository);

    auto expander = kr::ps::base::SuccessorExpander<p::GroundTag>(*task_context, sketch);
    auto context = expander.context_at(search_context->successor_generator->get_initial_node().get_state());
    const auto immediate = expander.labeled_successors(context);
    const auto accepted = expander.accepted_successors(context, immediate);
    ASSERT_GT(accepted.size(), 1);

    auto greedy_options = kr::ps::base::SketchSearchOptions<p::GroundTag> {};
    const auto greedy = kr::ps::base::find_solution(task_context, sketch, greedy_options);
    auto universal_options = kr::ps::base::SketchSearchOptions<p::GroundTag> {};
    universal_options.universal = true;
    const auto universal = kr::ps::base::find_solution(task_context, sketch, universal_options);
    ASSERT_TRUE(greedy.graph);
    ASSERT_TRUE(universal.graph);
    EXPECT_EQ(greedy.graph->get_out_degree(0), 1);
    EXPECT_EQ(universal.graph->get_out_degree(0), accepted.size());

    for (const auto edge : universal.graph->get_out_edge_indices(0))
    {
        const auto target_state = universal.graph->get_vertex(universal.graph->get_target(edge)).get_property().state.get_index();
        EXPECT_NE(std::ranges::find_if(immediate, [&](const auto& successor) { return successor.node.get_state().get_index() == target_state; }),
                  immediate.end());
    }

    auto options = kr::ps::base::SketchSearchOptions<p::GroundTag> {};
    options.max_num_states = 1;
    options.universal = true;
    const auto bounded = kr::ps::base::find_solution(task_context, sketch, options);
    EXPECT_EQ(bounded.status, kr::ps::base::SketchProofStatus::OUT_OF_STATES);
    ASSERT_TRUE(bounded.graph);
    EXPECT_EQ(bounded.graph->get_num_vertices(), 1);

    const auto two_step_only = kr::ps::base::dl::parse_sketch(
        read_fixture("kr/ps/base/executor/base_find_solution_uses_only_immediate_outcomes_and_universal_uses_all/two_step_only.sketch"),
        task->get_domain().get_domain(),
        *repository);
    auto two_step_expander = kr::ps::base::SuccessorExpander<p::GroundTag>(*task_context, two_step_only);
    auto two_step_context = two_step_expander.context_at(search_context->successor_generator->get_initial_node().get_state());
    const auto two_step_successors = two_step_expander.labeled_successors(two_step_context);
    EXPECT_TRUE(two_step_expander.accepted_successors(two_step_context, two_step_successors).empty());

    auto rejected_options = kr::ps::base::SketchSearchOptions<p::GroundTag> {};
    const auto rejected = kr::ps::base::find_solution(task_context, two_step_only, rejected_options);
    EXPECT_EQ(rejected.status, kr::ps::base::SketchProofStatus::FAILURE);
    ASSERT_TRUE(rejected.graph);
    EXPECT_EQ(rejected.graph->get_num_vertices(), 1);
    EXPECT_EQ(rejected.graph->get_num_edges(), 0);
    EXPECT_TRUE(rejected.deadend_states.empty());
    EXPECT_FALSE(rejected.open_states.empty());
}

}  // namespace runir::tests
