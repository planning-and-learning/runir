#include "fixtures.hpp"
#include "planning_fixtures.hpp"

#include <algorithm>
#include <chrono>
#include <concepts>
#include <cstdint>
#include <filesystem>
#include <gtest/gtest.h>
#include <ranges>
#include <runir/datasets/state_graph.hpp>
#include <runir/datasets/successors.hpp>
#include <runir/kr/database/syntax/query_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/base/detail/successor_expander_impl.hpp>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/dl/sketch_factory.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/sketch_executor.hpp>
#include <tyr/planning/axioms/axiom_evaluator.hpp>
#include <tyr/planning/states/state_repository.hpp>
#include <tyr/planning/successors/ground/successor_generator.hpp>
#include <tyr/planning/successors/lifted/successor_generator.hpp>
#include <tyr/planning/successors/successors.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/execution/onetbb.hpp>

namespace runir::tests
{

TEST(RunirTests, ProofStatusToString)
{
    using kr::ps::ProofStatus;
    EXPECT_EQ(kr::ps::to_string(ProofStatus::SUCCESS), "success");
    EXPECT_EQ(kr::ps::to_string(ProofStatus::FAILURE), "failure");
    EXPECT_EQ(kr::ps::to_string(ProofStatus::OUT_OF_TIME), "out_of_time");
    EXPECT_EQ(kr::ps::to_string(ProofStatus::OUT_OF_STATES), "out_of_states");
    EXPECT_THROW((void) kr::ps::to_string(static_cast<ProofStatus>(255)), std::invalid_argument);
}

TEST(RunirTests, FranceEtAlAaai2021SketchFactoriesExecuteOnExampleTasks)
{
    struct Case
    {
        std::filesystem::path domain;
        std::filesystem::path task;
        kr::ps::base::dl::SketchSpecification specification;
    };

    const auto cases = urd::Store<urd::sequence<Case>> {
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
        auto dl_repository = task_context->domain_context->base_repository->get_ptr<runir::kr::dl::ConstructorRepository>();
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
        EXPECT_GT(result.graph->num_vertices(), 0) << test_case.domain;

        auto search_options = kr::ps::base::SketchSearchOptions<tyr::GroundTag> {};
        const auto fragment = kr::ps::base::find_solution(task_context, sketch, search_options);
        EXPECT_TRUE(fragment.is_successful()) << test_case.domain;
        ASSERT_TRUE(fragment.graph) << test_case.domain;
        EXPECT_GT(fragment.graph->num_vertices(), 0) << test_case.domain;
        EXPECT_EQ(fragment.graph->num_edges() + 1, fragment.graph->num_vertices()) << test_case.domain;
        EXPECT_LE(fragment.graph->num_vertices(), result.graph->num_vertices()) << test_case.domain;
        ASSERT_TRUE(fragment.plan) << test_case.domain;
        const auto& plan = *fragment.plan;
        EXPECT_EQ(plan.length(), fragment.graph->num_edges()) << test_case.domain;
        auto& generator = *context->successor_generator;
        auto replay = runir::datasets::initial_node(*context);
        EXPECT_EQ(plan.start_node().state().index(), replay.state().pack().index());
        EXPECT_EQ(plan.start_node().metric(), replay.metric());
        for (const auto& step : plan.labeled_succ_nodes())
        {
            replay = runir::datasets::successor_node(*context, replay, step.label());
            EXPECT_EQ(step.node().state().index(), replay.state().pack().index());
            EXPECT_EQ(step.node().metric(), replay.metric());
        }
        EXPECT_EQ(plan.cost(), replay.metric());
        EXPECT_EQ(plan.cost(), plan.length());
        const auto& terminal = fragment.graph->vertex_value(fragment.graph->num_vertices() - 1);
        EXPECT_TRUE(terminal.is_goal);
        EXPECT_EQ(replay.state().pack().index(), terminal.state.index());

        auto expander = kr::ps::base::SuccessorExpander<tyr::GroundTag, kr::dl::semantics::FullEvaluationPolicy<tyr::GroundTag>>(*task_context, sketch);
        const auto state = runir::datasets::initial_node(*context).state();
        auto dl_context = expander.get_environment().make_dl_context(state);
        EXPECT_EQ(task_context->search_context.get(), context.get());
        EXPECT_EQ(task_context->search_context, context);
        EXPECT_EQ(dl_builder, &task_context->dl_builder);
        EXPECT_EQ(dl_builder, &dl_context.get_builder());
        EXPECT_EQ(dl_denotation_repository, task_context->dl_denotation_repository.get());
        // Evaluation owns reusable result storage; the task repository remains durable.
        EXPECT_NE(dl_denotation_repository, &dl_context.get_denotation_repository());
        EXPECT_EQ(dl_denotation_repository->get_ptr<tyr::formalism::Repository>(),
                  dl_context.get_denotation_repository().get_ptr<tyr::formalism::Repository>());
        EXPECT_EQ(&dl_context.get_caches(), &expander.get_environment().get_dl_caches());
    }
}

TEST(RunirTests, BaseMatchingDistinguishesCollidingStateIdsAcrossStorage)
{
    const auto search_context = make_gripper_ground_context();
    const auto task = search_context->task;
    const auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);
    const auto sketch = kr::ps::base::dl::parse_sketch(read_fixture("kr/ps/base/executor/any_transition.sketch"),
                                                       task->get_domain().get_domain(),
                                                       *task_context->domain_context->base_repository);
    auto expander = kr::ps::base::SuccessorExpander<tyr::GroundTag>(*task_context, sketch);
    auto& generator = *search_context->successor_generator;
    const auto initial = runir::datasets::initial_node(*search_context);
    const auto successors = runir::datasets::labeled_successor_nodes(*search_context, initial);
    const auto successor = std::ranges::find_if(successors, [&](const auto& candidate) { return candidate.node().state() != initial.state(); });
    ASSERT_NE(successor, successors.end());

    const auto target_repository = tyr::planning::StateRepository<tyr::GroundTag>::create(task);
    const auto source = initial.state().pack().unpack();
    const auto target = tyr::planning::share(*target_repository, tyr::planning::register_extended_state(*target_repository, successor->node().state()));
    ASSERT_EQ(source.pack().index(), target.pack().index());
    ASSERT_NE(source, target);
    EXPECT_TRUE(expander.matching_rule(source, target));
    EXPECT_FALSE(expander.matching_rule(source, source));
}

TEST(RunirTests, BaseFindSolutionUsesOnlyImmediateOutcomesAndUniversalUsesAll)
{
    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->base_repository->get_ptr<runir::kr::dl::ConstructorRepository>();
    auto repository = task_context->domain_context->base_repository;
    const auto sketch = kr::ps::base::dl::parse_sketch(read_fixture("kr/ps/base/executor/any_transition.sketch"), task->get_domain().get_domain(), *repository);

    auto expander = kr::ps::base::SuccessorExpander<tyr::GroundTag>(*task_context, sketch);
    auto& generator = *search_context->successor_generator;
    const auto initial = runir::datasets::initial_node(*search_context);
    const auto& state = initial.state();
    const auto immediate = runir::datasets::labeled_successor_nodes(*search_context, initial);
    auto num_accepted = static_cast<uint64_t>(0);
    auto statistics = kr::ps::base::SketchSearchStatistics {};
    EXPECT_TRUE(expander.for_each_successor(
        initial.state(),
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
    EXPECT_EQ(universal.status, kr::ps::ProofStatus::FAILURE);
    EXPECT_FALSE(universal.cycle.empty());
    EXPECT_EQ(greedy.graph->out_degree(0), 1);
    EXPECT_EQ(universal.graph->out_degree(0), num_accepted);

    for (const auto edge : universal.graph->out_edges(0))
    {
        const auto target_state = universal.graph->vertex_value(universal.graph->target(edge)).state.index();
        EXPECT_NE(std::ranges::find_if(immediate, [&](const auto& successor) { return successor.node().state().pack().index() == target_state; }),
                  immediate.end());
    }

    auto options = kr::ps::base::SketchSearchOptions<tyr::GroundTag> {};
    options.max_num_states = 1;
    options.universal = true;
    auto bounded_search = datasets::TaskSearchContext<tyr::GroundTag>::create(task, ygg::ExecutionContext::create(1));
    auto bounded_context = kr::TaskContext<tyr::GroundTag>::create(task_context->domain_context, bounded_search);
    const auto bounded = kr::ps::base::find_solution(bounded_context, sketch, options);
    EXPECT_EQ(bounded.status, kr::ps::ProofStatus::OUT_OF_STATES);
    EXPECT_FALSE(bounded.plan);
    ASSERT_TRUE(bounded.graph);
    EXPECT_EQ(bounded.graph->num_vertices(), 1);
    EXPECT_EQ(tyr::planning::num_states(*bounded_search->state_repository), 2);
    EXPECT_EQ(bounded.statistics.num_expanded, 1);
    // The first move stays in the current room: it is generated but rejected by sketch matching.
    EXPECT_EQ(bounded.statistics.num_generated, 2);
    const auto& packed_state = bounded.graph->vertex_value(0).state;
    EXPECT_EQ(packed_state.index(), state.pack().index());
    EXPECT_EQ(packed_state.unpack().pack(), packed_state);

    const auto two_step_only = kr::ps::base::dl::parse_sketch(
        read_fixture("kr/ps/base/executor/base_find_solution_uses_only_immediate_outcomes_and_universal_uses_all/two_step_only.sketch"),
        task->get_domain().get_domain(),
        *repository);
    auto two_step_expander = kr::ps::base::SuccessorExpander<tyr::GroundTag>(*task_context, two_step_only);
    auto rejected_statistics = kr::ps::base::SketchSearchStatistics {};
    EXPECT_TRUE(two_step_expander.for_each_successor(
        initial.state(),
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
    EXPECT_EQ(rejected.status, kr::ps::ProofStatus::FAILURE);
    EXPECT_FALSE(rejected.plan);
    ASSERT_TRUE(rejected.graph);
    EXPECT_EQ(rejected.graph->num_vertices(), 1);
    EXPECT_EQ(rejected.graph->num_edges(), 0);
    EXPECT_TRUE(rejected.deadend_states.empty());
    EXPECT_FALSE(rejected.open_states.empty());
    EXPECT_EQ(rejected.statistics.num_expanded, 1);
    EXPECT_EQ(rejected.statistics.num_generated, immediate.size());

    rejected_options.max_time = std::chrono::steady_clock::duration::zero();
    const auto timed_out = kr::ps::base::find_solution(task_context, two_step_only, rejected_options);
    EXPECT_EQ(timed_out.status, kr::ps::ProofStatus::OUT_OF_TIME);
    EXPECT_FALSE(timed_out.plan);
    EXPECT_EQ(timed_out.statistics.num_expanded, 0);
    EXPECT_EQ(timed_out.statistics.num_generated, 0);
}

namespace
{

template<tyr::TaskKind Kind>
void check_base_successor_early_stop(const datasets::TaskSearchContextPtr<Kind>& prototype)
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
    const auto initial = runir::datasets::initial_node(*search);
    const auto& state = initial.state();
    const auto count_bindings = [&]
    {
        size_t count = 0;
        for (const auto action : task->get_task().domain().actions())
            count += task->get_repository()->template size<ygg::Row<tyr::formalism::Action<tyr::LiftedTag>>>(action.index());
        return count;
    };
    const auto initial_binding_count = count_bindings();
    auto num_bindings = static_cast<size_t>(0);
    EXPECT_TRUE(search->successor_generator->for_each_applicable_action_binding(runir::datasets::node_of<Kind>(state),
                                                                                {},
                                                                                [&](auto)
                                                                                {
                                                                                    ++num_bindings;
                                                                                    return true;
                                                                                }));
    ASSERT_GT(num_bindings, 2);
    EXPECT_EQ(count_bindings(), initial_binding_count);
    auto num_emitted = static_cast<uint64_t>(0);
    auto statistics = kr::ps::base::SketchSearchStatistics {};
    const auto emit_one = [&](const auto& successor, auto rule)
    {
        ++num_emitted;
        EXPECT_GT(statistics.num_generated, 0);
        EXPECT_NE(successor.node().state().pack().index(), state.pack().index());
        EXPECT_EQ(rule, sketch.rules().front());
        return false;
    };

    EXPECT_FALSE(expander.for_each_successor(initial.state(), statistics, emit_one, [] { return true; }));
    EXPECT_EQ(statistics.num_generated, 0);
    EXPECT_EQ(num_emitted, 0);
    EXPECT_EQ(tyr::planning::num_states(*search->state_repository), 1);
    EXPECT_EQ(count_bindings(), initial_binding_count);

    auto stop_polls = 0;
    EXPECT_FALSE(expander.for_each_successor(initial.state(), statistics, emit_one, [&] { return ++stop_polls >= 2; }));
    EXPECT_EQ(statistics.num_generated, 0);
    EXPECT_EQ(num_emitted, 0);
    EXPECT_EQ(tyr::planning::num_states(*search->state_repository), 1);
    EXPECT_EQ(count_bindings(), initial_binding_count);

    auto rejected_search = datasets::TaskSearchContext<Kind>::create(task, ygg::ExecutionContext::create(1));
    auto rejected_context = kr::TaskContext<Kind>::create(domain_context, rejected_search);
    auto rejecting_expander = kr::ps::base::SuccessorExpander<Kind>(*rejected_context, rejecting_sketch);
    const auto rejected_initial = runir::datasets::initial_node(*rejected_search);
    auto rejected_statistics = kr::ps::base::SketchSearchStatistics {};
    EXPECT_FALSE(rejecting_expander.for_each_successor(
        rejected_initial.state(),
        rejected_statistics,
        [](const auto&, auto)
        {
            ADD_FAILURE() << "Unexpected accepted successor";
            return false;
        },
        [&] { return tyr::planning::num_states(*rejected_search->state_repository) > 1; }));
    EXPECT_GE(rejected_statistics.num_generated, 1);
    EXPECT_LT(rejected_statistics.num_generated, num_bindings);
    EXPECT_EQ(tyr::planning::num_states(*rejected_search->state_repository), 2);
    EXPECT_EQ(count_bindings(), initial_binding_count);
    rejected_statistics = {};
    EXPECT_TRUE(rejecting_expander.for_each_successor(
        rejected_initial.state(),
        rejected_statistics,
        [](const auto&, auto)
        {
            ADD_FAILURE() << "Unexpected accepted successor";
            return false;
        },
        [] { return false; }));
    EXPECT_EQ(rejected_statistics.num_generated, num_bindings);
    EXPECT_EQ(count_bindings(), initial_binding_count);

    EXPECT_FALSE(expander.for_each_successor(initial.state(), statistics, emit_one, [] { return false; }));
    const auto generated = statistics.num_generated;
    EXPECT_EQ(num_emitted, 1);
    EXPECT_GE(generated, 1);
    EXPECT_LT(generated, num_bindings);
    EXPECT_EQ(tyr::planning::num_states(*search->state_repository), 2);
    EXPECT_EQ(count_bindings(), initial_binding_count + (std::same_as<Kind, tyr::LiftedTag> ? 1 : 0));

    num_emitted = 0;
    EXPECT_TRUE(expander.for_each_successor(
        initial.state(),
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
}

}  // namespace

TEST(RunirTests, BaseGroundSuccessorsStopWithoutGeneratingAllStates) { check_base_successor_early_stop(make_gripper_ground_context()); }

TEST(RunirTests, BaseLiftedSuccessorsStopWithoutGeneratingAllStates)
{
    check_base_successor_early_stop(
        make_lifted_context(benchmark_path("classical/tests/gripper/domain.pddl"), benchmark_path("classical/tests/gripper/test-1.pddl")));
}

TEST(RunirTests, BaseSketchTransitionsRefreshDynamicQueriesAndReuseStaticQueries)
{
    const auto check = []<tyr::TaskKind Kind>()
    {
        const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../../support/data/kr/dl/query";
        datasets::TaskSearchContextPtr<Kind> search;
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            search = make_ground_context(directory / "domain.pddl", directory / "task.pddl");
        else
            search = make_lifted_context(directory / "domain.pddl", directory / "task.pddl");
        auto task_context = kr::TaskContext<Kind>::create(kr::DomainContext::create(search->task->get_domain()), search);
        const auto sketch = kr::ps::base::dl::parse_sketch(
            R"((:sketch
                (:features
                    (:boolean (:symbol ready) (:expression (b_atomic_state "ready" true)))
                    (:numerical (:symbol fixed) (:expression (n_count (q_atomic_state "fixed" (x y z)))))
                    (:numerical (:symbol remaining) (:expression (n_count (q_atomic_state "triple" (x y z))))))
                (:rules
                    (:rule (:symbol remove)
                        (:expression
                            (:conditions (positive ready) (greater_zero fixed) (greater_zero remaining))
                            (:effects (negative ready) (decreases remaining))))
                    (:rule (:symbol continue)
                        (:expression
                            (:conditions (negative ready) (greater_zero remaining))
                            (:effects (unchanged ready) (decreases remaining)))))))",
            search->task->get_domain().get_domain(),
            *task_context->domain_context->base_repository);

        auto expander = kr::ps::base::SuccessorExpander<Kind, kr::dl::semantics::FullEvaluationPolicy<Kind>>(*task_context, sketch);
        auto incremental = kr::ps::base::SuccessorExpander<Kind, kr::dl::semantics::DeltaEvaluationPolicy<Kind>>(*task_context, sketch);
        const auto matches = [&](const auto& source, const auto& target)
        {
            const auto full = expander.matching_rule(source, target);
            EXPECT_EQ(incremental.matching_rule(source, target), full);
            return full;
        };
        auto& generator = *search->successor_generator;
        const auto initial = runir::datasets::initial_node(*search);
        const auto& source = initial.state();
        const auto successors = runir::datasets::labeled_successor_nodes(*search, initial);
        ASSERT_GT(successors.size(), 1);
        ASSERT_TRUE(matches(source, successors.front().node().state()));

        const auto static_queries = expander.get_environment().get_dl_caches().get_queries(true);
        ASSERT_EQ(static_queries.size(), 1);
        const auto static_storage = std::vector(static_queries.front().rows().begin(), static_queries.front().rows().end());
        EXPECT_EQ(static_queries.front().rows().size(), 2);

        EXPECT_TRUE(matches(source, successors.back().node().state()));
        const auto target = successors.front().node().state();
        EXPECT_FALSE(matches(target, source));
        const auto next = runir::datasets::labeled_successor_nodes(*search, successors.front().node());
        ASSERT_FALSE(next.empty());
        EXPECT_TRUE(matches(target, next.front().node().state()));
        EXPECT_TRUE(matches(source, target));

        const auto later_static_queries = expander.get_environment().get_dl_caches().get_queries(true);
        ASSERT_EQ(later_static_queries.size(), 1);
        EXPECT_TRUE(std::ranges::equal(later_static_queries.front().rows(), static_storage));
    };
    check.template operator()<tyr::GroundTag>();
    check.template operator()<tyr::LiftedTag>();
}

}  // namespace runir::tests
