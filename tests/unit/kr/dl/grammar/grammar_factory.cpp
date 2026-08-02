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
#include <stdexcept>
#include <string>
#include <tyr/formalism/planning/parser.hpp>
#include <yggdrasil/serialization/json.hpp>

namespace runir::tests
{

TEST(RunirTests, RejectsUnknownGrammarFactorySpecification)
{
    namespace fp = tyr::formalism::planning;
    const auto planning_domain = fp::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain();
    const auto domain = planning_domain.get_domain();

    EXPECT_THROW(
        try {
            static_cast<void>(kr::dl::grammar::GrammarFactory::create_description(static_cast<kr::dl::grammar::GrammarSpecification>(999), domain));
        } catch (const std::runtime_error& error) {
            EXPECT_STREQ(error.what(), "Unknown grammar specification: 999.");
            throw;
        },
        std::runtime_error);
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

}  // namespace runir::tests
