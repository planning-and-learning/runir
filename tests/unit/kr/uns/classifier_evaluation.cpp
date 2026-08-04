#include "fixtures.hpp"
#include "planning_fixtures.hpp"

#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
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

    const auto state = search->state_repository->get_initial_state();
    auto builder = sem::Builder();
    auto denotation_repository = sem::DenotationRepositoryFactory().create(search->task->get_repository());
    auto context = sem::EvaluationContext<kr::UnsFamilyTag, tyr::GroundTag>(state, builder, denotation_repository);

    // some_ball is true and no_object is false, so the first clause (some_ball AND NOT no_object) holds.
    EXPECT_TRUE(kr::uns::classify(classifier, context));
}

}  // namespace runir::tests
