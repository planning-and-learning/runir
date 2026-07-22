#include "fixtures.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/errors.hpp>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/repository.hpp>
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

auto parse_gripper_task()
{
    const auto root = gripper_benchmark_root();
    return tyr::formalism::planning::Parser(root / "domain.pddl").parse_task(root / "test-1.pddl");
}

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

TEST(RunirTests, PolicyParserNegativeFixtures)
{
    const auto task = parse_gripper_task();
    const auto domain = task.get_domain().get_domain();
    auto base_dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(task.get_repository());
    auto base_repository = kr::ps::base::RepositoryFactory().create(base_dl_repository);
    auto ext_dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(task.get_repository());
    auto ext_repository = kr::ps::ext::RepositoryFactory().create(ext_dl_repository);
    const auto suite = load_fixture_json("kr/ps/parser_negative.json");
    const auto& cases = ygg::common::as_array(ygg::common::require_member(ygg::common::as_object(suite, "suite"), "cases", "suite"), "suite.cases");

    for (const auto& value : cases)
    {
        const auto& test_case = ygg::common::as_object(value, "case");
        const auto parser = ygg::common::as_string(test_case, "parser", "case");
        const auto description = read_fixture(ygg::common::as_string(test_case, "file", "case"));
        const auto error = ygg::common::as_string(test_case, "error", "case");
        expect_error_containing(
            [&]
            {
                if (parser == "base_sketch")
                    static_cast<void>(kr::ps::base::dl::parse_sketch(description, domain, *base_repository));
                else if (parser == "ext_module")
                    static_cast<void>(kr::ps::ext::dl::parse_module(description, domain, *ext_repository));
                else if (parser == "ext_program")
                    static_cast<void>(kr::ps::ext::dl::parse_module_program(description, domain, *ext_repository));
                else
                    throw std::runtime_error("Unknown parser fixture kind: " + parser);
            },
            error);
    }
}

TEST(RunirTests, PolicyParserSemanticErrorPointsAtExactIdentifier)
{
    const auto task = parse_gripper_task();
    const auto domain = task.get_domain().get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(task.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto description = read_fixture("kr/ps/parser/base_duplicate_feature.sketch");
    const auto first_same = description.find("same");
    ASSERT_NE(first_same, std::string::npos);
    const auto second_same = description.find("same", first_same + 1);
    ASSERT_NE(second_same, std::string::npos);
    const auto line_start = description.rfind('\n', second_same);
    ASSERT_NE(line_start, std::string::npos);
    const auto column = second_same - line_start - 1;

    try
    {
        static_cast<void>(kr::ps::base::dl::parse_sketch(description, domain, *repository));
        FAIL() << "Expected duplicate feature parsing to fail.";
    }
    catch (const kr::DuplicateDefinitionError& error)
    {
        const auto message = std::string(error.what());
        EXPECT_NE(message.find("In line 4:"), std::string::npos) << message;
        EXPECT_NE(message.find("(:numerical (:symbol same)"), std::string::npos) << message;
        EXPECT_NE(message.find(std::string(column, '_') + "^_"), std::string::npos) << message;
    }
}

TEST(RunirTests, ExtendedModuleFormatterPreservesAlternativeRuleGrouping)
{
    const auto task = parse_gripper_task();
    const auto domain = task.get_domain().get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::ExtFamilyTag>().create(task.get_repository());
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
