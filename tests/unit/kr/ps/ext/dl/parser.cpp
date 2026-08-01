#include "fixtures.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <stdexcept>
#include <string>
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

}  // namespace runir::tests
