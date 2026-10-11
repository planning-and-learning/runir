#include "fixtures.hpp"

#include <filesystem>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/ext/dl/incomplete_structural_termination.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/dl/structural_termination.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <tyr/formalism/planning/parser.hpp>
#include <variant>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::tests
{

namespace
{

auto gripper_domain() -> std::filesystem::path { return benchmark_path("classical/tests/gripper/domain.pddl"); }

}  // namespace

TEST(RunirTests, ExtIncompleteStructuralTerminationMapsNumericalOpponent)
{
    const auto planning_domain = tyr::formalism::planning::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto module_ =
        kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/incomplete_numerical_opponent.module"), planning_domain.get_domain(), *repository);

    const auto result = kr::ps::ext::dl::incomplete_structural_termination(module_);
    const auto numericals = module_.get_features<kr::dl::NumericalTag>();

    ASSERT_FALSE(result.is_terminating());
    ASSERT_EQ(numericals.size(), 1);
    ASSERT_EQ(result.surviving_rules.size(), 2);
    const auto& decreasing = result.surviving_rules.front();
    ASSERT_EQ(decreasing.blocking_reasons.size(), 1);
    const auto& reason = decreasing.blocking_reasons.front();
    ASSERT_TRUE(std::holds_alternative<kr::ps::ext::dl::NumericalFeatureView>(reason.feature));
    EXPECT_EQ(std::get<kr::ps::ext::dl::NumericalFeatureView>(reason.feature), numericals.front());
    ASSERT_EQ(reason.opposing_rules.size(), 1);
    EXPECT_EQ(reason.opposing_rules.front(), result.surviving_rules.back().rule);
}

TEST(RunirTests, ExtIncompleteStructuralTerminationUsesMemorySeparation)
{
    const auto planning_domain = tyr::formalism::planning::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto module_ = kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/separated_boolean.module"), planning_domain.get_domain(), *repository);

    const auto result = kr::ps::ext::dl::incomplete_structural_termination(module_);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_TRUE(result.surviving_rules.empty());
    EXPECT_TRUE(kr::ps::ext::dl::structural_termination(module_).is_terminating());
}

TEST(RunirTests, ExtIncompleteStructuralTerminationSeparatedRegisterLoadDoesNotBlockDecrease)
{
    const auto planning_domain = tyr::formalism::planning::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto module_ = kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/incomplete_register_load.module"), planning_domain.get_domain(), *repository);

    const auto result = kr::ps::ext::dl::incomplete_structural_termination(module_);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_EQ(result.surviving_rules.size(), 1);
    EXPECT_EQ(result.surviving_rules.front().rule.get_symbol(), "r2");
    EXPECT_TRUE(result.surviving_rules.front().blocking_reasons.empty());
}

TEST(RunirTests, ExtIncompleteStructuralTerminationHasNoFeatureLimit)
{
    const auto planning_domain = tyr::formalism::planning::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto module_ = kr::ps::ext::dl::parse_module(read_fixture("kr/ps/ext/dl/incomplete_many_features.module"), planning_domain.get_domain(), *repository);

    const auto result = kr::ps::ext::dl::incomplete_structural_termination(module_);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_EQ(module_.get_features<kr::dl::NumericalTag>().size(), 15);
}

TEST(RunirTests, ExtIncompleteStructuralTerminationAcyclicCallIsLocallyTerminating)
{
    const auto planning_domain = tyr::formalism::planning::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto program =
        kr::ps::ext::dl::parse_program(read_fixture("kr/ps/ext/dl/incomplete_acyclic_calls.program"), planning_domain.get_domain(), *repository);

    const auto result = kr::ps::ext::dl::incomplete_structural_termination(program);

    ASSERT_TRUE(result.is_terminating());
    ASSERT_EQ(result.module_results.size(), 2);
    for (const auto& module_result : result.module_results)
        EXPECT_TRUE(module_result.is_terminating());
    EXPECT_TRUE(result.recursive_call_rules.empty());
}

TEST(RunirTests, ExtIncompleteStructuralTerminationReportsRecursiveCalls)
{
    const auto planning_domain = tyr::formalism::planning::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto program =
        kr::ps::ext::dl::parse_program(read_fixture("kr/ps/ext/dl/incomplete_recursive_calls.program"), planning_domain.get_domain(), *repository);

    const auto result = kr::ps::ext::dl::incomplete_structural_termination(program);

    EXPECT_FALSE(result.is_terminating());
    EXPECT_EQ(result.module_results.size(), 2);
    for (const auto& module_result : result.module_results)
        EXPECT_TRUE(module_result.is_terminating());
    EXPECT_EQ(result.recursive_call_rules.size(), 2);
}
TEST(RunirTests, ExtIncompleteStructuralTerminationAcyclicEmptyProgramIsTerminating)
{
    const auto planning_domain = tyr::formalism::planning::Parser(gripper_domain()).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto program = kr::ps::ext::dl::parse_program(read_fixture("kr/ps/ext/dl/acyclic_empty.program"), planning_domain.get_domain(), *repository);

    const auto result = kr::ps::ext::dl::incomplete_structural_termination(program);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_EQ(result.module_results.size(), 2);
    EXPECT_TRUE(result.recursive_call_rules.empty());
}

}  // namespace runir::tests
