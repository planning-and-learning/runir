#include "fixtures.hpp"
#include "planning_fixtures.hpp"

#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/unsolvability.hpp>
#include <runir/kr/uns/classify.hpp>
#include <runir/kr/uns/dl/parser.hpp>
#include <runir/kr/uns/repository.hpp>

namespace runir::tests
{

TEST(RunirTests, UnsClassifierClassifies)
{
    namespace sem = kr::dl::semantics;

    auto search = make_gripper_ground_context();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::UnsFamilyTag>().create(search->task->get_repository());
    auto repository = kr::uns::RepositoryFactory().create(dl_repository);
    const auto classifier = kr::uns::dl::parse_classifier(read_fixture("kr/uns/positive.classifier"), search->task->get_domain().get_domain(), *repository);

    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);
    auto builder = sem::Builder();
    auto denotation_repository = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = sem::EvaluationStorage<kr::UnsFamilyTag>(denotation_repository);
    auto context = sem::StateEvaluationContext<kr::UnsFamilyTag, tyr::GroundTag>(state, builder, storage);

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
    auto full = kr::ps::ClassifierUnsolvability<Kind, kr::dl::semantics::FullEvaluationPolicy<kr::UnsFamilyTag, Kind>>(*task_context, classifier);
    auto delta = kr::ps::ClassifierUnsolvability<Kind, kr::dl::semantics::DeltaEvaluationPolicy<kr::UnsFamilyTag, Kind>>(*task_context, classifier);
    auto& generator = *search->successor_generator;
    const auto initial = generator.get_initial_node(*search->state_repository, *search->axiom_evaluator);
    EXPECT_FALSE(full.is_unsolvable(initial.get_state()));
    EXPECT_FALSE(delta.is_unsolvable(initial.get_state()));

    auto matches = 0;
    EXPECT_TRUE(generator.for_each_applicable_action_binding(
        initial,
        [&](auto binding)
        {
            const auto successor = generator.get_successor_node(initial, binding, *search->state_repository, *search->axiom_evaluator);
            const auto expected = full.is_unsolvable(successor.get_state());
            matches += expected;
            EXPECT_EQ(delta.is_unsolvable(successor.get_state()), expected);
            EXPECT_FALSE(delta.is_unsolvable(initial.get_state()));
            EXPECT_EQ(delta.is_unsolvable(successor.get_state()), expected);
            return true;
        }));
    EXPECT_GT(matches, 0);
}

}  // namespace

TEST(RunirTests, UnsGroundClassifierEvaluationPoliciesAgreeAfterBacktracking)
{
    check_classifier_evaluation_policies(make_gripper_ground_context());
}

TEST(RunirTests, UnsLiftedClassifierEvaluationPoliciesAgreeAfterBacktracking)
{
    check_classifier_evaluation_policies(
        make_lifted_context(benchmark_path("classical/tests/gripper/domain.pddl"), benchmark_path("classical/tests/gripper/test-1.pddl")));
}

}  // namespace runir::tests
