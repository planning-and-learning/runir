#include "fixtures.hpp"
#include "planning_fixtures.hpp"

#include <chrono>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/sketch_executor.hpp>
#include <runir/kr/task_context.hpp>
#include <runir/kr/uns/classify.hpp>
#include <runir/kr/uns/dl/parser.hpp>
#include <runir/kr/uns/repository.hpp>

namespace runir::tests
{

namespace
{

template<tyr::TaskKind Kind>
void check_classifier_failures(datasets::TaskSearchContextPtr<Kind> search_context)
{
    auto task = search_context->task;
    auto task_context = kr::TaskContext<Kind>::create(kr::DomainContext::create(task->get_domain()), search_context);

    auto dl_repository = task_context->domain_context->base_repository->get_dl_repository_ptr();
    auto repository = task_context->domain_context->base_repository;
    const auto sketch = kr::ps::base::dl::parse_sketch(read_fixture("kr/ps/base/executor/any_transition.sketch"), task->get_domain().get_domain(), *repository);

    auto classifier_dl_repository = task_context->domain_context->uns_repository->get_dl_repository_ptr();
    auto classifier_repository = task_context->domain_context->uns_repository;
    const auto classifier = kr::uns::dl::parse_classifier(read_fixture("kr/uns/always.classifier"), task->get_domain().get_domain(), *classifier_repository);

    auto options = kr::ps::base::SketchSearchOptions<Kind> {};
    options.classifier = classifier;
    const auto result = kr::ps::base::find_solution(task_context, sketch, options);

    EXPECT_EQ(result.status, kr::ps::base::SketchProofStatus::FAILURE);
    EXPECT_EQ(result.statistics.num_expanded, 0);
    EXPECT_EQ(result.statistics.num_generated, 0);
    ASSERT_TRUE(result.graph);
    ASSERT_EQ(result.graph->get_num_vertices(), 1);
    EXPECT_EQ(result.graph->get_num_edges(), 0);
    ASSERT_EQ(result.deadend_states.size(), 1);
    EXPECT_TRUE(result.open_states.empty());
    const auto& label = result.graph->get_vertex(result.deadend_states.front()).get_property();
    EXPECT_FALSE(label.is_goal);
    EXPECT_FALSE(label.is_alive);
    EXPECT_TRUE(label.is_unsolvable);
    EXPECT_GT(task_context->dl_denotation_repository->template size<kr::dl::semantics::Denotation<kr::dl::BooleanTag>>(), 0);

    options.max_time = std::chrono::steady_clock::duration::zero();
    const auto timed_out = kr::ps::base::find_solution(task_context, sketch, options);
    EXPECT_EQ(timed_out.status, kr::ps::base::SketchProofStatus::OUT_OF_TIME);
    EXPECT_TRUE(timed_out.deadend_states.empty());
    EXPECT_TRUE(timed_out.open_states.empty());
    ASSERT_TRUE(timed_out.graph);
    ASSERT_EQ(timed_out.graph->get_num_vertices(), 1);
    EXPECT_TRUE(timed_out.graph->get_vertex(0).get_property().is_unsolvable);
    options.max_time.reset();

    const auto goal_classifier = kr::uns::dl::parse_classifier(
        read_fixture("kr/ps/base/executor/base_find_solution_treats_classifier_matches_as_terminal_failures/goal_classifier.classifier"),
        task->get_domain().get_domain(),
        *classifier_repository);
    options.universal = true;
    options.classifier = goal_classifier;
    const auto goal_result = kr::ps::base::find_solution(task_context, sketch, options);
    auto found_goal = false;
    auto caches = kr::dl::semantics::DenotationCaches<kr::UnsFamilyTag>();
    for (const auto vertex : goal_result.graph->get_vertex_indices())
    {
        const auto& goal_label = goal_result.graph->get_vertex(vertex).get_property();
        if (!goal_label.is_goal)
            continue;
        auto context = kr::dl::semantics::StateEvaluationContext<kr::UnsFamilyTag, Kind>(goal_label.state.unpack(),
                                                                                     task_context->dl_builder,
                                                                                     *task_context->dl_denotation_repository,
                                                                                     task_context->dl_builder.get_workspace(),
                                                                                     caches);
        caches.clear(false);
        EXPECT_TRUE(kr::uns::classify(goal_classifier, context));
        EXPECT_FALSE(goal_label.is_unsolvable);
        found_goal = true;
    }
    EXPECT_TRUE(found_goal);
}

}  // namespace

TEST(RunirTests, BaseFindSolutionTreatsClassifierMatchesAsTerminalFailures)
{
    check_classifier_failures(make_gripper_ground_context());
}

TEST(RunirTests, BaseLiftedFindSolutionTreatsClassifierMatchesAsTerminalFailures)
{
    check_classifier_failures(
        make_lifted_context(benchmark_path("classical/tests/gripper/domain.pddl"), benchmark_path("classical/tests/gripper/test-1.pddl")));
}

}  // namespace runir::tests
