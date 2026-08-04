#include "fixtures.hpp"
#include "planning_fixtures.hpp"

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

TEST(RunirTests, BaseFindSolutionTreatsClassifierMatchesAsTerminalFailures)
{
    namespace p = tyr::planning;

    auto search_context = make_gripper_ground_context();
    auto task = search_context->task;
    auto task_context = kr::TaskContext<tyr::GroundTag>::create(search_context);

    auto dl_repository = task_context->base_dl_repository;
    auto repository = task_context->base_repository;
    const auto sketch = kr::ps::base::dl::parse_sketch(read_fixture("kr/ps/base/executor/any_transition.sketch"), task->get_domain().get_domain(), *repository);

    auto classifier_dl_repository = task_context->uns_dl_repository;
    auto classifier_repository = task_context->uns_repository;
    const auto classifier = kr::uns::dl::parse_classifier(read_fixture("kr/uns/always.classifier"), task->get_domain().get_domain(), *classifier_repository);

    auto options = kr::ps::base::SketchSearchOptions<tyr::GroundTag> {};
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
        auto context = kr::dl::semantics::EvaluationContext<kr::UnsFamilyTag, tyr::GroundTag>(goal_label.state,
                                                                                            task_context->dl_builder,
                                                                                            *task_context->dl_denotation_repository);
        EXPECT_TRUE(kr::uns::classify(goal_classifier, context));
        EXPECT_FALSE(goal_label.is_unsolvable);
        found_goal = true;
    }
    EXPECT_TRUE(found_goal);
}

}  // namespace runir::tests
