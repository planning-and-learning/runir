#include "tyr/common/json_loader.hpp"

#include <boost/json.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <iostream>
#include <runir/datasets/config.hpp>
#include <runir/datasets/equivalence_graph.hpp>
#include <runir/datasets/formatter.hpp>
#include <string>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/planning.hpp>
#include <vector>

namespace runir::tests
{

namespace
{

struct EquivalenceGraphFigureCase
{
    std::string name;
    std::filesystem::path domain_file;
    std::vector<std::filesystem::path> task_files;
    datasets::EquivalencePolicyMode equivalence_policy;
    std::size_t expected_num_vertices = 0;
    std::size_t expected_num_edges = 0;
};

auto parse_equivalence_policy_mode(std::string_view value) -> datasets::EquivalencePolicyMode
{
    if (value == "identity")
        return datasets::EquivalencePolicyMode::IDENTITY;
    if (value == "gi")
        return datasets::EquivalencePolicyMode::GI;
    throw std::runtime_error(fmt::format("Unsupported equivalence policy mode: {}", value));
}

EquivalenceGraphFigureCase parse_case(const boost::json::object& suite, const boost::json::object& object)
{
    auto task_files = std::vector<std::filesystem::path> {};
    for (const auto& task_file : tyr::common::as_array(*object.if_contains("task_files"), "case.task_files"))
        task_files.push_back(tyr::common::suite_path(suite, std::string(task_file.as_string())));

    return EquivalenceGraphFigureCase { tyr::common::as_string(object, "name", "case"),
                                        tyr::common::suite_path(suite, tyr::common::as_string(object, "domain_file", "case")),
                                        std::move(task_files),
                                        parse_equivalence_policy_mode(tyr::common::as_string(object, "equivalence_policy", "case")),
                                        tyr::common::as_size(object, "expected_num_vertices", "case"),
                                        tyr::common::as_size(object, "expected_num_edges", "case") };
}

std::vector<EquivalenceGraphFigureCase> load_cases()
{
    const auto suite = tyr::common::load_json_file(tyr::common::root_path() / "tests/unit/datasets/equivalence_graph.json");
    const auto& suite_object = tyr::common::as_object(suite, "suite");
    const auto* tests_value = suite_object.if_contains("tests");
    if (!tests_value)
        throw std::runtime_error("suite.tests is required.");

    auto result = std::vector<EquivalenceGraphFigureCase> {};
    for (const auto& case_value : tyr::common::as_array(*tests_value, "suite.tests"))
        result.push_back(parse_case(suite_object, tyr::common::as_object(case_value, "case")));
    return result;
}

}  // namespace

class EquivalenceGraphTest : public ::testing::TestWithParam<EquivalenceGraphFigureCase>
{
};

TEST_P(EquivalenceGraphTest, MatchesExpectedProperties)
{
    namespace fp = tyr::formalism::planning;
    namespace p = tyr::planning;

    const auto& param = GetParam();
    auto parser = fp::Parser(param.domain_file);
    auto execution_context = tyr::ExecutionContext::create(1);
    auto contexts = datasets::TaskSearchContextList<p::GroundTag> {};
    contexts.reserve(param.task_files.size());

    for (const auto& task_file : param.task_files)
    {
        const auto planning_task = parser.parse_task(task_file);
        auto lifted_task = p::Task<p::LiftedTag>(planning_task);
        auto task = lifted_task.instantiate_ground_task(*execution_context).task;
        contexts.emplace_back(task, execution_context);
    }

    const auto result = datasets::generate_equivalence_graph(contexts, param.equivalence_policy);
    const auto figure = fmt::format("{}", *result.graph);

    std::cout << "EQUIVALENCE_GRAPH_FIGURE " << param.name << "\n" << figure;
    EXPECT_EQ(result.graph->get_forward_graph().get_num_vertices(), param.expected_num_vertices);
    EXPECT_EQ(result.graph->get_forward_graph().get_num_edges(), param.expected_num_edges);
}

INSTANTIATE_TEST_SUITE_P(RunirDatasets,
                         EquivalenceGraphTest,
                         ::testing::ValuesIn(load_cases()),
                         [](const testing::TestParamInfo<EquivalenceGraphFigureCase>& info) { return info.param.name; });

}  // namespace runir::tests
