#include "fixtures.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/dl/sketch_factory.hpp>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/syntactic_complexity.hpp>
#include <stdexcept>
#include <string_view>
#include <tyr/formalism/planning/parser.hpp>

namespace runir::tests
{

TEST(RunirTests, FranceEtAlAaai2021SketchFactoriesParse)
{
    namespace fp = tyr::formalism::planning;

    struct Case
    {
        std::string_view domain;
        kr::ps::base::dl::SketchSpecification specification;
    };

    const Case cases[] {
        { "classical/tests/gripper/domain.pddl", kr::ps::base::dl::SketchSpecification::GRIPPER_FRANCE_ET_AL_AAAI2021 },
        { "classical/tests/blocks_3/domain.pddl", kr::ps::base::dl::SketchSpecification::BLOCKS3OPS_FRANCE_ET_AL_AAAI2021 },
        { "classical/tests/spanner/domain.pddl", kr::ps::base::dl::SketchSpecification::SPANNER_FRANCE_ET_AL_AAAI2021 },
        { "classical/tests/delivery/domain.pddl", kr::ps::base::dl::SketchSpecification::DELIVERY_FRANCE_ET_AL_AAAI2021 },
    };

    auto dl_repository_factory = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>();
    auto repository_factory = kr::ps::base::RepositoryFactory();
    for (const auto& test_case : cases)
    {
        const auto planning_domain = fp::Parser(benchmark_path(test_case.domain)).get_domain();
        auto dl_repository = dl_repository_factory.create(planning_domain.get_repository());
        auto repository = repository_factory.create(dl_repository);
        const auto sketch = kr::ps::base::dl::SketchFactory::create(test_case.specification, planning_domain.get_domain(), *repository);

        EXPECT_EQ(sketch.get_index(), ygg::Index<kr::ps::base::Sketch>(0));
        EXPECT_EQ(repository->template size<kr::ps::base::Sketch>(), 1);
        const auto formatted = fmt::format("{}", sketch);
        EXPECT_EQ(fmt::format("{}", kr::ps::base::dl::parse_sketch(formatted, planning_domain.get_domain(), *repository)), formatted);
    }
}

TEST(RunirTests, RejectsUnknownSketchFactorySpecification)
{
    EXPECT_THROW(
        try {
            static_cast<void>(kr::ps::base::dl::SketchFactory::create_description(static_cast<kr::ps::base::dl::SketchSpecification>(999)));
        } catch (const std::runtime_error& error) {
            EXPECT_STREQ(error.what(), "Unknown sketch specification: 999.");
            throw;
        },
        std::runtime_error);
}

TEST(RunirTests, PolicySketchParserParsesConditionsAndEffects)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);

    const auto sketch = kr::ps::base::dl::parse_sketch(read_fixture("kr/dl/grammar/policy_parser.sketch"), planning_domain.get_domain(), *repository);

    EXPECT_EQ(sketch.get_index(), ygg::Index<kr::ps::base::Sketch>(0));
    EXPECT_EQ(repository->template size<kr::ps::base::Sketch>(), 1);
    EXPECT_EQ(repository->template size<kr::ps::base::Rule>(), 2);
    EXPECT_EQ((repository->template size<kr::ps::Feature<kr::BaseFamilyTag, kr::ps::dl::BooleanFeature>>()), 2);
    EXPECT_EQ((repository->template size<kr::ps::Feature<kr::BaseFamilyTag, kr::ps::dl::NumericalFeature>>()), 1);
    EXPECT_EQ(repository->template size<kr::ps::ConditionVariant<kr::BaseFamilyTag>>(), 4);
    EXPECT_EQ(repository->template size<kr::ps::EffectVariant<kr::BaseFamilyTag>>(), 5);

    const auto first_rule = sketch.get_rules()[0];
    EXPECT_EQ(first_rule.get_symbol(), "ready-rule");
    EXPECT_EQ(fmt::format("{}", first_rule.get_conditions().front()), "(positive r)");
    EXPECT_EQ(fmt::format("{}", first_rule.get_effects().front()), "(negative r)");

    const auto formatted = fmt::format("{}", sketch);
    EXPECT_EQ(fmt::format("{}", kr::ps::base::dl::parse_sketch(formatted, planning_domain.get_domain(), *repository)), formatted);
    const auto feature = sketch.template get_features<kr::ps::dl::BooleanFeature>().front();
    const auto concrete_complexity = ygg::visit([](auto concrete) { return kr::ps::base::dl::syntactic_complexity(concrete); }, feature.get_variant());
    EXPECT_EQ(kr::ps::base::syntactic_complexity(feature), concrete_complexity);
    EXPECT_EQ(concrete_complexity, kr::dl::semantics::syntactic_complexity(feature.get_expression()));
    EXPECT_EQ(kr::ps::base::syntactic_complexity(sketch), 6);
}

}  // namespace runir::tests
