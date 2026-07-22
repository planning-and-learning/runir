#include "fixtures.hpp"

#include <algorithm>
#include <filesystem>
#include <gtest/gtest.h>
#include <runir/datasets/config.hpp>
#include <runir/datasets/state_graph.hpp>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/dl/sketch_factory.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/sketch_executor.hpp>
#include <runir/kr/ps/base/successor_expander.hpp>
#include <runir/kr/uns/classify.hpp>
#include <runir/kr/uns/dl/parser.hpp>
#include <runir/kr/uns/repository.hpp>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <vector>
#include <yggdrasil/execution/onetbb.hpp>

namespace runir::tests
{

namespace
{

std::filesystem::path benchmark_prefix() { return std::filesystem::path(BENCHMARKS_DIR); }

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

    for (const auto& test_case : cases)
    {
        auto parser = fp::Parser(test_case.domain);
        const auto planning_task = parser.parse_task(test_case.task);
        auto execution_context = ygg::ExecutionContext::create(1);
        auto lifted_task = p::Task<p::LiftedTag>(planning_task);
        auto task = lifted_task.instantiate_ground_task(*execution_context).task;
        auto context = datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
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
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
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

TEST(RunirTests, BaseFindSolutionTreatsClassifierMatchesAsTerminalFailures)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto domain = benchmark_prefix() / "classical" / "tests" / "gripper" / "domain.pddl";
    const auto task_file = benchmark_prefix() / "classical" / "tests" / "gripper" / "test-1.pddl";
    const auto planning_task = fp::Parser(domain).parse_task(task_file);
    auto execution_context = ygg::ExecutionContext::create(1);
    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    auto search_context = datasets::TaskSearchContext<p::GroundTag>::create(task, execution_context);
    auto task_context = kr::TaskContext<p::GroundTag>::create(search_context);

    auto dl_repository = task_context->base_dl_repository;
    auto repository = task_context->base_repository;
    const auto sketch = kr::ps::base::dl::parse_sketch(read_fixture("kr/ps/base/executor/any_transition.sketch"), task->get_domain().get_domain(), *repository);

    auto classifier_dl_repository = task_context->uns_dl_repository;
    auto classifier_repository = task_context->uns_repository;
    const auto classifier = kr::uns::dl::parse_classifier(read_fixture("kr/uns/always.classifier"), task->get_domain().get_domain(), *classifier_repository);

    auto options = kr::ps::base::SketchSearchOptions<p::GroundTag> {};
    options.classifier = classifier;
    const auto result = kr::ps::base::find_solution(task_context, sketch, options);

    EXPECT_EQ(result.status, kr::ps::base::SketchProofStatus::FAILURE);
    ASSERT_TRUE(result.graph);
    ASSERT_EQ(result.graph->get_num_vertices(), 1);
    EXPECT_EQ(result.graph->get_num_edges(), 0);
    ASSERT_EQ(result.deadend_states.size(), 1);
    EXPECT_TRUE(result.open_states.empty());
    const auto& label = result.graph->get_vertex(result.deadend_states.front()).get_property();
    EXPECT_FALSE(label.is_goal);
    EXPECT_FALSE(label.is_alive);
    EXPECT_TRUE(label.is_unsolvable);
    EXPECT_GT(task_context->dl_denotation_repository->size<kr::dl::BooleanTag>(), 0);

    const auto goal_classifier = kr::uns::dl::parse_classifier(
        read_fixture("kr/ps/base/executor/base_find_solution_treats_classifier_matches_as_terminal_failures/goal_classifier.classifier"),
        task->get_domain().get_domain(),
        *classifier_repository);
    options.universal = true;
    options.classifier = goal_classifier;
    const auto goal_result = kr::ps::base::find_solution(task_context, sketch, options);
    auto found_goal = false;
    for (const auto vertex : goal_result.graph->get_vertex_indices())
    {
        const auto& goal_label = goal_result.graph->get_vertex(vertex).get_property();
        if (!goal_label.is_goal)
            continue;
        auto context = kr::dl::semantics::EvaluationContext<kr::UnsFamilyTag, p::GroundTag>(goal_label.state,
                                                                                            task_context->dl_builder,
                                                                                            *task_context->dl_denotation_repository);
        EXPECT_TRUE(kr::uns::classify(goal_classifier, context));
        EXPECT_FALSE(goal_label.is_unsolvable);
        found_goal = true;
    }
    EXPECT_TRUE(found_goal);
}

}  // namespace runir::tests
