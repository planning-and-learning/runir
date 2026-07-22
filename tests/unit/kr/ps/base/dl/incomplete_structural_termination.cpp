#include "fixtures.hpp"

#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/base/dl/incomplete_structural_termination.hpp>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/dl/sketch_factory.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <tyr/formalism/planning/parser.hpp>
#include <variant>

namespace runir::tests
{

TEST(RunirTests, IncompleteStructuralTerminationBarmanSketchIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto sketch = kr::ps::base::dl::parse_sketch(read_fixture("kr/ps/base/dl/barman.sketch"), planning_domain.get_domain(), *repository);

    EXPECT_TRUE(kr::ps::base::dl::incomplete_structural_termination(sketch).is_terminating());
}

TEST(RunirTests, IncompleteStructuralTerminationRequiresR3MarkedComplementaryConditions)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto sketch = kr::ps::base::dl::parse_sketch(read_fixture("kr/ps/base/dl/r3_complementary.sketch"), planning_domain.get_domain(), *repository);

    EXPECT_TRUE(kr::ps::base::dl::incomplete_structural_termination(sketch).is_terminating());
}

TEST(RunirTests, IncompleteStructuralTerminationBooleanOscillatorReportsBlockingReasons)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto sketch = kr::ps::base::dl::parse_sketch(read_fixture("kr/ps/base/dl/oscillator.sketch"), planning_domain.get_domain(), *repository);

    const auto result = kr::ps::base::dl::incomplete_structural_termination(sketch);
    const auto booleans = sketch.get_features<kr::ps::dl::BooleanFeature>();
    ASSERT_FALSE(result.is_terminating());
    ASSERT_EQ(result.surviving_rules.size(), 2);
    ASSERT_EQ(booleans.size(), 1);
    for (const auto& surviving : result.surviving_rules)
    {
        ASSERT_EQ(surviving.blocking_reasons.size(), 1);
        const auto& reason = surviving.blocking_reasons.front();
        ASSERT_TRUE(std::holds_alternative<kr::ps::base::dl::BooleanFeatureView>(reason.feature));
        EXPECT_EQ(std::get<kr::ps::base::dl::BooleanFeatureView>(reason.feature).get_index(), booleans.front().get_index());
        ASSERT_EQ(reason.opposing_rules.size(), 1);
        EXPECT_NE(reason.opposing_rules.front().get_index(), surviving.rule.get_index());
    }
}

TEST(RunirTests, IncompleteStructuralTerminationEmptySketchIsTerminating)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto sketch = kr::ps::base::dl::SketchFactory::create_empty(*repository);

    EXPECT_TRUE(kr::ps::base::dl::incomplete_structural_termination(sketch).is_terminating());
}

}  // namespace runir::tests
