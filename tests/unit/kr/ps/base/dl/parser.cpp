#include "fixtures.hpp"

#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/errors.hpp>
#include <runir/kr/ps/base/dl/parser.hpp>
#include <runir/kr/ps/base/repository.hpp>
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

TEST(RunirTests, PolicyBaseParserNegativeFixtures)
{
    const auto planning_domain = parse_gripper_domain();
    const auto domain = planning_domain.get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
    auto repository = kr::ps::base::RepositoryFactory().create(dl_repository);
    const auto suite = load_fixture_json("kr/ps/parser_negative.json");
    const auto& cases = ygg::common::as_array(ygg::common::require_member(ygg::common::as_object(suite, "suite"), "cases", "suite"), "suite.cases");

    for (const auto& value : cases)
    {
        const auto& test_case = ygg::common::as_object(value, "case");
        if (ygg::common::as_string(test_case, "parser", "case") != "base_sketch")
            continue;

        const auto description = read_fixture(ygg::common::as_string(test_case, "file", "case"));
        const auto error = ygg::common::as_string(test_case, "error", "case");
        expect_error_containing([&] { static_cast<void>(kr::ps::base::dl::parse_sketch(description, domain, *repository)); }, error);
    }
}

TEST(RunirTests, PolicyParserSemanticErrorPointsAtExactIdentifier)
{
    const auto planning_domain = parse_gripper_domain();
    const auto domain = planning_domain.get_domain();
    auto dl_repository = kr::dl::ConstructorRepositoryFactoryFor<kr::BaseFamilyTag>().create(planning_domain.get_repository());
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

}  // namespace runir::tests
