#include "fixtures.hpp"
#include "planning_fixtures.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
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
        auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), context);
        EXPECT_THROW(kr::TaskContext<tyr::GroundTag>::create(task_context->domain_context, datasets::TaskSearchContextPtr<tyr::GroundTag> {}),
                     std::invalid_argument);
        EXPECT_THROW(kr::TaskContext<tyr::GroundTag>::create(kr::DomainContextPtr {}, context), std::invalid_argument);
        auto dl_repository = task_context->domain_context->base_repository->get_dl_repository_ptr();
        auto repository = task_context->domain_context->base_repository;
        const auto sketch = kr::ps::base::dl::SketchFactory::create(test_case.specification, task->get_domain().get_domain(), *repository);
        const auto* dl_builder = &task_context->dl_builder;
        const auto* dl_denotation_repository = task_context->dl_denotation_repository.get();
        auto proof_options = kr::ps::base::SketchSearchOptions<tyr::GroundTag> {};
        proof_options.universal = true;
        const auto result = kr::ps::base::find_solution(task_context, sketch, proof_options);

        EXPECT_TRUE(result.is_successful()) << test_case.domain;
        EXPECT_TRUE(result.deadend_states.empty()) << test_case.domain;
        EXPECT_TRUE(result.open_states.empty()) << test_case.domain;
        EXPECT_TRUE(result.cycle.empty()) << test_case.domain;
        EXPECT_FALSE(result.plan) << test_case.domain;
        EXPECT_GT(result.graph->get_num_vertices(), 0) << test_case.domain;

        auto search_options = kr::ps::base::SketchSearchOptions<tyr::GroundTag> {};
        const auto fragment = kr::ps::base::find_solution(task_context, sketch, search_options);
        EXPECT_TRUE(fragment.is_successful()) << test_case.domain;
        ASSERT_TRUE(fragment.graph) << test_case.domain;
        EXPECT_GT(fragment.graph->get_num_vertices(), 0) << test_case.domain;
        EXPECT_EQ(fragment.graph->get_num_edges() + 1, fragment.graph->get_num_vertices()) << test_case.domain;
        EXPECT_LE(fragment.graph->get_num_vertices(), result.graph->get_num_vertices()) << test_case.domain;
        ASSERT_TRUE(fragment.plan) << test_case.domain;
        const auto& plan = *fragment.plan;
        EXPECT_EQ(plan.get_length(), fragment.graph->get_num_edges()) << test_case.domain;
        auto& generator = *context->successor_generator;
        auto replay = generator.get_initial_node(*context->state_repository, *context->axiom_evaluator);
        EXPECT_EQ(plan.get_start_node().get_state().get_index(), replay.get_state().get_index());
        EXPECT_EQ(plan.get_start_node().get_metric(), replay.get_metric());
        for (const auto& step : plan.get_labeled_succ_nodes())
        {
            replay = generator.get_successor_node(replay, step.label, *context->state_repository, *context->axiom_evaluator);
            EXPECT_EQ(step.node.get_state().get_index(), replay.get_state().get_index());
            EXPECT_EQ(step.node.get_metric(), replay.get_metric());
        }
        EXPECT_EQ(plan.get_cost(), replay.get_metric());
        EXPECT_EQ(plan.get_cost(), plan.get_length());
        const auto& terminal = fragment.graph->get_vertex(fragment.graph->get_num_vertices() - 1).get_property();
        EXPECT_TRUE(terminal.is_goal);
        EXPECT_EQ(replay.get_state().get_index(), terminal.state.get_index());

        auto expander = kr::ps::base::SuccessorExpander<tyr::GroundTag>(*task_context, sketch);
        const auto state = context->successor_generator->get_initial_node(*context->state_repository, *context->axiom_evaluator).get_state();
        auto dl_context = expander.get_environment().make_dl_context(state);
        EXPECT_EQ(task_context->search_context.get(), context.get());
        EXPECT_EQ(task_context->search_context, context);
        EXPECT_EQ(dl_builder, &task_context->dl_builder);
        EXPECT_EQ(dl_builder, &dl_context.get_builder());
        EXPECT_EQ(dl_denotation_repository, task_context->dl_denotation_repository.get());
        EXPECT_EQ(dl_denotation_repository, &dl_context.get_denotation_repository());
    }
}

TEST(RunirTests, BaseFindSolutionUsesOnlyImmediateOutcomesAndUniversalUsesAll)
{
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->base_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->base_repository;
    const auto sketch = kr::ps::base::dl::parse_sketch(read_fixture("kr/ps/base/executor/any_transition.sketch"), task->get_domain().get_domain(), *repository);

    auto expander = kr::ps::base::SuccessorExpander<tyr::GroundTag>(*task_context, sketch);
    auto& generator = *search_context->successor_generator;
    const auto initial = generator.get_initial_node(*search_context->state_repository, *search_context->axiom_evaluator);
    const auto state = initial.get_state();
    const auto immediate = generator.get_labeled_successor_nodes(initial, *search_context->state_repository, *search_context->axiom_evaluator);
    auto num_accepted = uint64_t(0);
    auto statistics = kr::ps::base::SketchSearchStatistics {};
    EXPECT_TRUE(expander.for_each_successor(
        initial.get_state(),
        statistics,
        [&](const auto&, auto)
        {
            ++num_accepted;
            return true;
        },
        [] { return false; }));
    EXPECT_EQ(statistics.num_generated, immediate.size());
    ASSERT_GT(num_accepted, 1);

    auto greedy_options = kr::ps::base::SketchSearchOptions<tyr::GroundTag> {};
    const auto greedy = kr::ps::base::find_solution(task_context, sketch, greedy_options);
    auto universal_options = kr::ps::base::SketchSearchOptions<tyr::GroundTag> {};
    universal_options.universal = true;
    const auto universal = kr::ps::base::find_solution(task_context, sketch, universal_options);
    ASSERT_TRUE(greedy.graph);
    ASSERT_TRUE(universal.graph);
    // Moving between rooms and back closes a cycle before either state satisfies the goal.
    EXPECT_EQ(universal.status, kr::ps::base::SketchProofStatus::FAILURE);
    EXPECT_FALSE(universal.cycle.empty());
    EXPECT_EQ(greedy.graph->get_out_degree(0), 1);
    EXPECT_EQ(universal.graph->get_out_degree(0), num_accepted);

    for (const auto edge : universal.graph->get_out_edge_indices(0))
    {
        const auto target_state = universal.graph->get_vertex(universal.graph->get_target(edge)).get_property().state.get_index();
        EXPECT_NE(std::ranges::find_if(immediate, [&](const auto& successor) { return successor.node.get_state().get_index() == target_state; }),
                  immediate.end());
    }

    auto options = kr::ps::base::SketchSearchOptions<tyr::GroundTag> {};
    options.max_num_states = 1;
    options.universal = true;
    auto bounded_search = datasets::TaskSearchContext<tyr::GroundTag>::create(task, ygg::ExecutionContext::create(1));
    auto bounded_context = kr::TaskContext<tyr::GroundTag>::create(task_context->domain_context, bounded_search);
    const auto bounded = kr::ps::base::find_solution(bounded_context, sketch, options);
    EXPECT_EQ(bounded.status, kr::ps::base::SketchProofStatus::OUT_OF_STATES);
    EXPECT_FALSE(bounded.plan);
    ASSERT_TRUE(bounded.graph);
    EXPECT_EQ(bounded.graph->get_num_vertices(), 1);
    EXPECT_EQ(bounded_search->state_repository->num_states(), 2);
    EXPECT_EQ(bounded.statistics.num_expanded, 1);
    // The first move stays in the current room: it is generated but rejected by sketch matching.
    EXPECT_EQ(bounded.statistics.num_generated, 2);
    const auto& packed_state = bounded.graph->get_vertex(0).get_property().state;
    EXPECT_EQ(packed_state.get_index(), state.get_index());
    EXPECT_EQ(packed_state.unpack().pack(), packed_state);

    const auto two_step_only = kr::ps::base::dl::parse_sketch(
        read_fixture("kr/ps/base/executor/base_find_solution_uses_only_immediate_outcomes_and_universal_uses_all/two_step_only.sketch"),
        task->get_domain().get_domain(),
        *repository);
    auto two_step_expander = kr::ps::base::SuccessorExpander<tyr::GroundTag>(*task_context, two_step_only);
    auto rejected_statistics = kr::ps::base::SketchSearchStatistics {};
    EXPECT_TRUE(two_step_expander.for_each_successor(
        initial.get_state(),
        rejected_statistics,
        [](const auto&, auto)
        {
            ADD_FAILURE() << "Unexpected accepted successor";
            return false;
        },
        [] { return false; }));
    EXPECT_EQ(rejected_statistics.num_generated, immediate.size());

    auto rejected_options = kr::ps::base::SketchSearchOptions<tyr::GroundTag> {};
    const auto rejected = kr::ps::base::find_solution(task_context, two_step_only, rejected_options);
    EXPECT_EQ(rejected.status, kr::ps::base::SketchProofStatus::FAILURE);
    EXPECT_FALSE(rejected.plan);
    ASSERT_TRUE(rejected.graph);
    EXPECT_EQ(rejected.graph->get_num_vertices(), 1);
    EXPECT_EQ(rejected.graph->get_num_edges(), 0);
    EXPECT_TRUE(rejected.deadend_states.empty());
    EXPECT_FALSE(rejected.open_states.empty());
    EXPECT_EQ(rejected.statistics.num_expanded, 1);
    EXPECT_EQ(rejected.statistics.num_generated, immediate.size());

    rejected_options.max_time = std::chrono::steady_clock::duration::zero();
    const auto timed_out = kr::ps::base::find_solution(task_context, two_step_only, rejected_options);
    EXPECT_EQ(timed_out.status, kr::ps::base::SketchProofStatus::OUT_OF_TIME);
    EXPECT_FALSE(timed_out.plan);
    EXPECT_EQ(timed_out.statistics.num_expanded, 0);
    EXPECT_EQ(timed_out.statistics.num_generated, 0);
}

namespace
{

template<tyr::TaskKind Kind>
void check_base_successor_early_stop(datasets::TaskSearchContextPtr<Kind> prototype)
{
    const auto task = prototype->task;
    const auto domain_context = kr::DomainContext::create(task->get_domain());
    const auto sketch = kr::ps::base::dl::parse_sketch(read_fixture("kr/ps/base/executor/any_transition.sketch"),
                                                    task->get_domain().get_domain(),
                                                    *domain_context->base_repository);
    const auto rejecting_sketch = kr::ps::base::dl::parse_sketch(
        read_fixture("kr/ps/base/executor/base_find_solution_uses_only_immediate_outcomes_and_universal_uses_all/two_step_only.sketch"),
        task->get_domain().get_domain(),
        *domain_context->base_repository);

    auto search = datasets::TaskSearchContext<Kind>::create(task, ygg::ExecutionContext::create(1));
    auto context = kr::TaskContext<Kind>::create(domain_context, search);
    auto expander = kr::ps::base::SuccessorExpander<Kind>(*context, sketch);
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto state = initial.get_state();
    const auto num_bindings = search->successor_generator->get_applicable_action_bindings(initial).size();
    ASSERT_GT(num_bindings, 2);
    auto num_emitted = uint64_t(0);
    auto statistics = kr::ps::base::SketchSearchStatistics {};
    const auto emit_one = [&](const auto& successor, auto rule)
    {
        ++num_emitted;
        EXPECT_GT(statistics.num_generated, 0);
        EXPECT_NE(successor.node.get_state().get_index(), state.get_index());
        EXPECT_EQ(rule, sketch.get_rules().front());
        return false;
    };

    EXPECT_FALSE(expander.for_each_successor(initial.get_state(), statistics, emit_one, [] { return true; }));
    EXPECT_EQ(statistics.num_generated, 0);
    EXPECT_EQ(num_emitted, 0);
    EXPECT_EQ(search->state_repository->num_states(), 1);

    auto stop_polls = 0;
    EXPECT_FALSE(expander.for_each_successor(initial.get_state(), statistics, emit_one, [&] { return ++stop_polls >= 2; }));
    EXPECT_EQ(statistics.num_generated, 0);
    EXPECT_EQ(num_emitted, 0);
    EXPECT_EQ(search->state_repository->num_states(), 1);

    EXPECT_FALSE(expander.for_each_successor(initial.get_state(), statistics, emit_one, [] { return false; }));
    const auto generated = statistics.num_generated;
    EXPECT_EQ(num_emitted, 1);
    EXPECT_GE(generated, 1);
    EXPECT_LT(generated, num_bindings);
    EXPECT_EQ(search->state_repository->num_states(), 2);

    num_emitted = 0;
    EXPECT_TRUE(expander.for_each_successor(
        initial.get_state(),
        statistics,
        [&](const auto&, auto)
        {
            ++num_emitted;
            return true;
        },
        [] { return false; }));
    EXPECT_EQ(statistics.num_generated, generated + num_bindings);
    EXPECT_EQ(statistics.num_expanded, 0);
    EXPECT_GT(num_emitted, 1);
    EXPECT_LT(num_emitted, num_bindings);  // The self-transition counts as generated, but is rejected.

    auto rejected_search = datasets::TaskSearchContext<Kind>::create(task, ygg::ExecutionContext::create(1));
    auto rejected_context = kr::TaskContext<Kind>::create(domain_context, rejected_search);
    auto rejecting_expander = kr::ps::base::SuccessorExpander<Kind>(*rejected_context, rejecting_sketch);
    const auto rejected_initial =
        rejected_search->successor_generator->get_initial_node(*rejected_search->state_repository, *rejected_search->axiom_evaluator);
    auto rejected_statistics = kr::ps::base::SketchSearchStatistics {};
    EXPECT_FALSE(rejecting_expander.for_each_successor(
        rejected_initial.get_state(),
        rejected_statistics,
        [](const auto&, auto)
        {
            ADD_FAILURE() << "Unexpected accepted successor";
            return false;
        },
        [&] { return rejected_search->state_repository->num_states() > 1; }));
    EXPECT_GE(rejected_statistics.num_generated, 1);
    EXPECT_LT(rejected_statistics.num_generated, num_bindings);
    EXPECT_EQ(rejected_search->state_repository->num_states(), 2);
}

}  // namespace

TEST(RunirTests, BaseGroundSuccessorsStopWithoutGeneratingAllStates) { check_base_successor_early_stop(make_gripper_ground_context()); }

TEST(RunirTests, BaseLiftedSuccessorsStopWithoutGeneratingAllStates)
{
    check_base_successor_early_stop(make_lifted_context(benchmark_path("classical/tests/gripper/domain.pddl"),
                                                       benchmark_path("classical/tests/gripper/test-1.pddl")));
}

TEST(RunirTests, BaseSketchTransitionsRefreshDynamicQueriesAndReuseStaticQueries)
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../fixtures/kr/dl/query";
    auto search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(search->task->get_domain()), search);
    const auto sketch = kr::ps::base::dl::parse_sketch(
        R"((:sketch
            (:features
                (:numerical (:symbol fixed) (:expression (n_count (q_atomic_state "fixed" (x y z)))))
                (:numerical (:symbol remaining) (:expression (n_count (q_atomic_state "triple" (x y z))))))
            (:rules
                (:rule (:symbol remove)
                    (:expression
                        (:conditions (greater_zero fixed) (greater_zero remaining))
                        (:effects (decreases remaining)))))))",
        search->task->get_domain().get_domain(),
        *task_context->domain_context->base_repository);

    auto expander = kr::ps::base::SuccessorExpander<tyr::GroundTag>(*task_context, sketch);
    auto& generator = *search->successor_generator;
    const auto initial = generator.get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto source = initial.get_state();
    const auto successors = generator.get_labeled_successor_nodes(initial, *search->state_repository, *search->axiom_evaluator);
    ASSERT_GT(successors.size(), 1);
    ASSERT_TRUE(expander.matching_rule(source, successors.front().node.get_state()));

    auto& static_queries = expander.get_environment().get_dl_caches().get_queries(true);
    ASSERT_EQ(static_queries.size(), 1);
    const auto* static_storage = &static_queries.begin()->second.storage();
    EXPECT_EQ(static_queries.begin()->second.size(), 2);

    EXPECT_TRUE(expander.matching_rule(source, successors.back().node.get_state()));
    const auto target = successors.front().node.get_state();
    EXPECT_FALSE(expander.matching_rule(target, source));
    const auto next = generator.get_labeled_successor_nodes(successors.front().node, *search->state_repository, *search->axiom_evaluator);
    ASSERT_FALSE(next.empty());
    EXPECT_TRUE(expander.matching_rule(target, next.front().node.get_state()));
    EXPECT_TRUE(expander.matching_rule(source, target));

    ASSERT_EQ(static_queries.size(), 1);
    EXPECT_EQ(&static_queries.begin()->second.storage(), static_storage);
}

}  // namespace runir::tests
