#include "fixtures.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/formatter.hpp>
#include <runir/kr/dl/cnf_grammar/translate.hpp>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/formatter.hpp>
#include <runir/kr/dl/grammar/grammar_factory.hpp>
#include <runir/kr/dl/grammar/parser.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/dl/sketch_factory.hpp>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/syntactic_complexity.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tyr/formalism/planning/parser.hpp>
#include <vector>
#include <yggdrasil/semantics/comparison.hpp>
#include <yggdrasil/serialization/json.hpp>

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

    const auto cases = std::vector<Case> {
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

TEST(RunirTests, RejectsUnknownFactorySpecifications)
{
    namespace fp = tyr::formalism::planning;
    const auto domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain().get_domain();

    EXPECT_THROW(
        try {
            static_cast<void>(kr::ps::base::dl::SketchFactory::create_description(static_cast<kr::ps::base::dl::SketchSpecification>(999)));
        } catch (const std::runtime_error& error) {
            EXPECT_STREQ(error.what(), "Unknown sketch specification: 999.");
            throw;
        },
        std::runtime_error);
    EXPECT_THROW(
        try {
            static_cast<void>(kr::dl::grammar::GrammarFactory::create_description(static_cast<kr::dl::grammar::GrammarSpecification>(999), domain));
        } catch (const std::runtime_error& error) {
            EXPECT_STREQ(error.what(), "Unknown grammar specification: 999.");
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

TEST(RunirTests, GrammarFactoryFixturesMatch)
{
    namespace fp = tyr::formalism::planning;
    using GrammarTag = kr::dl::grammar::GrammarTag<kr::BaseFamilyTag>;
    const auto suite = load_fixture_json("kr/dl/grammar/factory.json");
    const auto& cases = ygg::common::as_array(ygg::common::require_member(ygg::common::as_object(suite, "suite"), "cases", "suite"), "suite.cases");

    for (const auto& value : cases)
    {
        const auto& test_case = ygg::common::as_object(value, "case");
        const auto planning_domain = fp::Parser(benchmark_path(ygg::common::as_string(test_case, "domain_file", "case"))).get_domain();
        const auto domain = planning_domain.get_domain();
        auto repository = kr::dl::grammar::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
        const auto grammar = kr::dl::grammar::GrammarFactory::create(kr::dl::grammar::GrammarSpecification::FRANCE_ET_AL_AAAI2021, domain, *repository);
        const auto formatted = fmt::format("{}", grammar);
        EXPECT_EQ(formatted, read_fixture(ygg::common::as_string(test_case, "grammar_file", "case")));
        const auto reparsed_grammar = kr::dl::grammar::parse_grammar(formatted, domain, *repository);
        EXPECT_EQ(grammar, reparsed_grammar);
        EXPECT_EQ(grammar.get_index(), ygg::Index<GrammarTag>(0));
        EXPECT_EQ(reparsed_grammar.get_index(), grammar.get_index());
        EXPECT_EQ(grammar.get_context().template size<GrammarTag>(), 1);

        auto cnf_repository = kr::dl::cnf_grammar::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
        const auto cnf = kr::dl::cnf_grammar::translate(grammar, *cnf_repository);
        const auto cnf_formatted = fmt::format("{}", cnf);
        EXPECT_EQ(cnf_formatted, read_fixture(ygg::common::as_string(test_case, "cnf_file", "case")));
        const auto reparsed = kr::dl::grammar::parse_grammar(cnf_formatted, domain, *repository);
        EXPECT_EQ(cnf, kr::dl::cnf_grammar::translate(reparsed, *cnf_repository));
        EXPECT_EQ(reparsed.get_index(), ygg::Index<GrammarTag>(1));
        EXPECT_EQ(grammar.get_context().template size<GrammarTag>(), 2);
    }
}

TEST(RunirTests, GrammarFactoryHandlesBooleanPrimitivePredicates)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/ferry/domain.pddl")).get_domain();
    const auto domain = planning_domain.get_domain();
    auto repository = kr::dl::grammar::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    const auto grammar = kr::dl::grammar::GrammarFactory::create(kr::dl::grammar::GrammarSpecification::FRANCE_ET_AL_AAAI2021, domain, *repository);
    const auto formatted = fmt::format("{}", grammar);

    EXPECT_NE(formatted.find("(b_atomic_state \"empty-ferry\" true)"), std::string::npos);
    EXPECT_NE(formatted.find("(b_atomic_state \"empty-ferry\" false)"), std::string::npos);
    EXPECT_EQ(grammar, kr::dl::grammar::parse_grammar(formatted, domain, *repository));

    auto cnf_repository = kr::dl::cnf_grammar::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    const auto cnf = kr::dl::cnf_grammar::translate(grammar, *cnf_repository);
    const auto cnf_formatted = fmt::format("{}", cnf);
    const auto reparsed = kr::dl::grammar::parse_grammar(cnf_formatted, domain, *repository);
    EXPECT_EQ(cnf, kr::dl::cnf_grammar::translate(reparsed, *cnf_repository));
}

TEST(RunirTests, ExtGrammarNumericReferencesFormatWithoutExternalNames)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    auto grammar_repository = kr::dl::grammar::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto cnf_repository = kr::dl::cnf_grammar::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());

    const auto format_grammar = [&]<typename T>(auto identifier)
    {
        auto data = ygg::Data<T>(identifier);
        kr::dl::grammar::canonicalize(data);
        return fmt::format("{}", grammar_repository->get_or_create(data).first);
    };
    const auto format_cnf = [&]<typename T>(auto identifier)
    {
        auto data = ygg::Data<T>(identifier);
        kr::dl::cnf_grammar::canonicalize(data);
        return fmt::format("{}", cnf_repository->get_or_create(data).first);
    };

    using ConceptRegister = kr::dl::RegisterIdentifier<kr::dl::ConceptTag>;
    using RoleRegister = kr::dl::RegisterIdentifier<kr::dl::RoleTag>;
    using ConceptArgument = kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>;
    using RoleArgument = kr::dl::ArgumentIdentifier<kr::dl::RoleTag>;
    using BooleanArgument = kr::dl::ArgumentIdentifier<kr::dl::BooleanTag>;
    using NumericalArgument = kr::dl::ArgumentIdentifier<kr::dl::NumericalTag>;

    EXPECT_EQ((format_grammar.template operator()<kr::dl::grammar::Concept<kr::ExtFamilyTag, kr::dl::RegisterTag>>(ConceptRegister(1))), "(c_register 1)");
    EXPECT_EQ((format_grammar.template operator()<kr::dl::grammar::Role<kr::ExtFamilyTag, kr::dl::RegisterTag>>(RoleRegister(2))), "(r_register 2)");
    EXPECT_EQ((format_grammar.template operator()<kr::dl::grammar::Concept<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::ConceptTag>>>(ConceptArgument(3))),
              "(c_argument 3)");
    EXPECT_EQ((format_grammar.template operator()<kr::dl::grammar::Role<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::RoleTag>>>(RoleArgument(4))),
              "(r_argument 4)");
    EXPECT_EQ((format_grammar.template operator()<kr::dl::grammar::Boolean<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::BooleanTag>>>(BooleanArgument(5))),
              "(b_argument 5)");
    EXPECT_EQ(
        (format_grammar.template operator()<kr::dl::grammar::Numerical<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::NumericalTag>>>(NumericalArgument(6))),
        "(n_argument 6)");
    EXPECT_EQ((format_cnf.template operator()<kr::dl::cnf_grammar::Concept<kr::ExtFamilyTag, kr::dl::RegisterTag>>(ConceptRegister(1))), "(c_register 1)");
    EXPECT_EQ((format_cnf.template operator()<kr::dl::cnf_grammar::Role<kr::ExtFamilyTag, kr::dl::RegisterTag>>(RoleRegister(2))), "(r_register 2)");
    EXPECT_EQ((format_cnf.template operator()<kr::dl::cnf_grammar::Concept<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::ConceptTag>>>(ConceptArgument(3))),
              "(c_argument 3)");
    EXPECT_EQ((format_cnf.template operator()<kr::dl::cnf_grammar::Role<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::RoleTag>>>(RoleArgument(4))),
              "(r_argument 4)");
    EXPECT_EQ((format_cnf.template operator()<kr::dl::cnf_grammar::Boolean<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::BooleanTag>>>(BooleanArgument(5))),
              "(b_argument 5)");
    EXPECT_EQ(
        (format_cnf.template operator()<kr::dl::cnf_grammar::Numerical<kr::ExtFamilyTag, kr::dl::ArgumentTag<kr::dl::NumericalTag>>>(NumericalArgument(6))),
        "(n_argument 6)");
}

}  // namespace runir::tests
