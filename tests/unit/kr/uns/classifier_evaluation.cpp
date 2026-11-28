#include "fixtures.hpp"
#include "planning_fixtures.hpp"

#include <gtest/gtest.h>
#include <runir/datasets/successors.hpp>
#include <runir/kr/database/syntax/query_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/unsolvability.hpp>
#include <runir/kr/uns/classify.hpp>
#include <runir/kr/uns/dl/parser.hpp>
#include <runir/kr/uns/repository.hpp>
#include <tyr/planning/successors/ground/successor_generator.hpp>
#include <tyr/planning/successors/lifted/successor_generator.hpp>
#include <tyr/planning/successors/successors.hpp>

namespace runir::tests
{

TEST(RunirTests, UnsClassifierClassifies)
{
    auto search = make_gripper_ground_context();
    auto dl_repository = std::make_shared<kr::dl::ConstructorRepository>(search->task->get_repository());
    auto repository = std::make_shared<kr::uns::Repository>(dl_repository);
    const auto classifier = kr::uns::dl::parse_classifier(read_fixture("kr/uns/positive.classifier"), search->task->get_domain().get_domain(), *repository);

    const auto state = runir::datasets::initial_node(*search).state();
    auto builder = kr::dl::semantics::Builder();
    auto denotation_repository = kr::dl::semantics::DenotationRepository(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotation_repository);
    auto context = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>(state, builder, storage);

    // some_ball is true and no_object is false, so the first clause (some_ball AND NOT no_object) holds.
    EXPECT_TRUE(kr::uns::classify<tyr::GroundTag>(classifier, context));
}

namespace
{

template<tyr::TaskKind Kind>
void check_classifier_evaluation_policies(datasets::TaskSearchContextPtr<Kind> search)
{
    auto task_context = kr::TaskContext<Kind>::create(kr::DomainContext::create(search->task->get_domain()), search);
    const auto classifier = kr::uns::dl::parse_classifier(
        R"((:classifier
            (:symbol carrying)
            (:features
                (:boolean (:symbol carry) (:expression (b_nonempty (c_some (r_atomic_state "carry") (c_top))))))
            (:expression (or (and carry)))))",
        search->task->get_domain().get_domain(),
        *task_context->domain_context->uns_repository);
    auto full = kr::ps::ClassifierUnsolvability<Kind, kr::dl::semantics::FullEvaluationPolicy<Kind>>(*task_context, classifier);
    auto delta = kr::ps::ClassifierUnsolvability<Kind, kr::dl::semantics::DeltaEvaluationPolicy<Kind>>(*task_context, classifier);
    auto& generator = *search->successor_generator;
    const auto initial = runir::datasets::initial_node(*search);
    EXPECT_FALSE(full.is_unsolvable(initial.state()));
    EXPECT_FALSE(delta.is_unsolvable(initial.state()));

    auto matches = 0;
    EXPECT_TRUE(generator.for_each_applicable_action_binding(runir::datasets::node_of<Kind>(initial.state()),
                                                             {},
                                                             [&](auto binding)
                                                             {
                                                                 const auto successor = runir::datasets::successor_node(*search, initial, binding);
                                                                 const auto expected = full.is_unsolvable(successor.state());
                                                                 matches += expected;
                                                                 EXPECT_EQ(delta.is_unsolvable(successor.state()), expected);
                                                                 EXPECT_FALSE(delta.is_unsolvable(initial.state()));
                                                                 EXPECT_EQ(delta.is_unsolvable(successor.state()), expected);
                                                                 return true;
                                                             }));
    EXPECT_GT(matches, 0);
}

}  // namespace

TEST(RunirTests, UnsGroundClassifierEvaluationPoliciesAgreeAfterBacktracking) { check_classifier_evaluation_policies(make_gripper_ground_context()); }

TEST(RunirTests, UnsLiftedClassifierEvaluationPoliciesAgreeAfterBacktracking)
{
    check_classifier_evaluation_policies(
        make_lifted_context(benchmark_path("classical/tests/gripper/domain.pddl"), benchmark_path("classical/tests/gripper/test-1.pddl")));
}

}  // namespace runir::tests
