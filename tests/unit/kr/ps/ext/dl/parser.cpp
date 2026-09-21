#include "fixtures.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/errors.hpp>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/parser.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <tyr/formalism/planning/parser.hpp>
#include <yggdrasil/serialization/json.hpp>

namespace runir::tests
{
namespace
{

auto parse_gripper_domain() { return tyr::formalism::planning::Parser(benchmark_path("classical/tests/gripper/domain.pddl")).get_domain(); }

template<typename Parse>
void expect_error_containing(Parse&& parse, const std::string& expected)
{
    try
    {
        parse();
        FAIL() << "Expected parsing to fail.";
    }
    catch (const std::runtime_error& error)
    {
        EXPECT_NE(std::string(error.what()).find(expected), std::string::npos) << error.what();
    }
}

}  // namespace

TEST(RunirTests, PolicyExtParserNegativeFixtures)
{
    const auto planning_domain = parse_gripper_domain();
    const auto domain = planning_domain.get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto suite = load_fixture_json("kr/ps/parser_negative.json");
    const auto& cases = ygg::common::as_array(ygg::common::require_member(ygg::common::as_object(suite, "suite"), "cases", "suite"), "suite.cases");

    for (const auto& value : cases)
    {
        const auto& test_case = ygg::common::as_object(value, "case");
        const auto parser = ygg::common::as_string(test_case, "parser", "case");
        if (parser == "base_sketch")
            continue;

        const auto description = read_fixture(ygg::common::as_string(test_case, "file", "case"));
        const auto error = ygg::common::as_string(test_case, "error", "case");
        expect_error_containing(
            [&]
            {
                if (parser == "ext_module")
                    static_cast<void>(kr::ps::ext::dl::parse_module(description, domain, *repository));
                else if (parser == "ext_program")
                    static_cast<void>(kr::ps::ext::dl::parse_module_program(description, domain, *repository));
                else
                    throw std::runtime_error("Unknown parser fixture kind: " + parser);
            },
            error);
    }
}

TEST(RunirTests, ExtendedModuleFormatterPreservesAlternativeRuleGrouping)
{
    const auto planning_domain = parse_gripper_domain();
    const auto domain = planning_domain.get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::ext::RepositoryFactory().create(dl_repository);
    const auto module = kr::ps::ext::dl::parse_module(read_fixture("kr/ps/parser/alternative_rules.module"), domain, *repository);

    ASSERT_EQ(module.get_memory_transitions().size(), 1);
    ASSERT_EQ(module.get_memory_transitions().front().size(), 2);
    const auto formatted = fmt::format("{}", module);
    const auto symbol = std::string("(:symbol choose)");
    ASSERT_NE(formatted.find(symbol), std::string::npos) << formatted;
    EXPECT_EQ(formatted.find(symbol), formatted.rfind(symbol)) << formatted;

    const auto reparsed = kr::ps::ext::dl::parse_module(formatted, domain, *repository);
    ASSERT_EQ(reparsed.get_memory_transitions().size(), 1);
    EXPECT_EQ(reparsed.get_memory_transitions().front().size(), 2);
    EXPECT_EQ(fmt::format("{}", reparsed), formatted);
}

TEST(RunirTests, RelationalExpressionsRoundTripAndRejectGenerationGrammars)
{
    const auto planning_domain = parse_gripper_domain();
    const auto domain = planning_domain.get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(planning_domain.get_repository());
    auto grammar_repository = kr::dl::grammar::BaseConstructorRepositoryFactory().create(planning_domain.get_repository());

    const std::vector<std::string> expressions {
        R"((n_count (q_project () (q_atomic_state "at" (X Y)))))",
        R"((n_count (q_atomic_goal "at" false (X Y))))",
        R"((n_count (q_join (q_rename (X) (q_concept a (c_top))) (q_concept Y (c_top)))))",
        R"((n_count (q_union (q_concept X (c_top)) (q_concept X (c_bot)))))",
        R"((n_count (q_difference (q_concept X (c_top)) (q_concept X (c_bot)))))",
        R"((n_count (c_project Y (q_atomic_state "at" (X Y)))))",
        R"((n_count (r_project Y X (q_atomic_state "at" (X Y)))))"
    };
    for (const auto& expression : expressions)
    {
        const auto parsed = kr::ps::ext::dl::parse_numerical(expression, domain, *repository);
        const auto formatted = fmt::format("{}", parsed);
        EXPECT_EQ(kr::ps::ext::dl::parse_numerical(formatted, domain, *repository), parsed);
        expect_error_containing(
            [&] { kr::dl::grammar::parse_grammar("((n_0 " + expression + "))", domain, *grammar_repository); },
            "Relational expressions are not supported in generation grammars");
    }

    const auto boolean = kr::ps::ext::dl::parse_boolean(
        "(b_nonempty (q_select_equal X Y (q_role (X Y) (r_identity (c_top)))))", domain, *repository);
    EXPECT_EQ(kr::ps::ext::dl::parse_boolean(fmt::format("{}", boolean), domain, *repository), boolean);
    expect_error_containing(
        [&] { kr::dl::grammar::parse_grammar("((b_0 (b_nonempty (q_concept X (c_top)))))", domain, *grammar_repository); },
        "Relational expressions are not supported in generation grammars");

    EXPECT_THROW(kr::ps::ext::dl::parse_numerical("(n_count (q_atomic_state \"at\" (X X)))", domain, *repository), kr::InvalidExpressionError);
    EXPECT_THROW(kr::ps::ext::dl::parse_numerical("(n_count (q_project (missing) (q_concept X (c_top))))", domain, *repository), kr::InvalidExpressionError);
    for (const auto& expression : {
             "(n_count (c_project missing (q_concept X (c_top))))",
             "(n_count (r_project X missing (q_role (X Y) (r_universal))))",
             "(n_count (q_select_equal X missing (q_concept X (c_top))))" })
    {
        try
        {
            kr::ps::ext::dl::parse_numerical(expression, domain, *repository);
            FAIL() << "Expected an unknown query column to fail.";
        }
        catch (const kr::InvalidExpressionError& error)
        {
            EXPECT_TRUE(error.diagnostic().location.has_value()) << error.what();
            EXPECT_NE(error.message().find("unknown column label"), std::string::npos) << error.what();
        }
    }
    EXPECT_THROW(kr::ps::ext::dl::parse_numerical("(n_count (q_concept X c_0))", domain, *repository), kr::InvalidExpressionError);
    EXPECT_THROW(kr::ps::ext::dl::parse_numerical("(n_count (q_select_value X \"missing\" (q_concept X (c_top))))", domain, *repository), kr::UndefinedSymbolError);
}

}  // namespace runir::tests
