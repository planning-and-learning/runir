#include "fixtures.hpp"

#include <boost/json.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <iostream>
#include <runir/datasets/equivalence_graph.hpp>
#include <runir/datasets/formatter.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <yggdrasil/serialization/json_loader.hpp>

namespace runir::tests
{

static_assert(datasets::StateGraphVertexRef(0, 1) == datasets::StateGraphVertexRef(0, 1));
static_assert(datasets::StateGraphVertexRef(0, 1) != datasets::StateGraphVertexRef(0, 2));
static_assert(datasets::StateGraphVertexRef(0, 1) < datasets::StateGraphVertexRef(0, 2));
static_assert(datasets::StateGraphVertexRef(0, 1) <= datasets::StateGraphVertexRef(0, 1));
static_assert(datasets::StateGraphVertexRef(0, 2) > datasets::StateGraphVertexRef(0, 1));
static_assert(datasets::StateGraphVertexRef(0, 2) >= datasets::StateGraphVertexRef(0, 2));

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

EquivalenceGraphFigureCase parse_case(const boost::json::object& object)
{
    auto task_files = std::vector<std::filesystem::path> {};
    for (const auto& task_file : ygg::common::as_array(*object.if_contains("task_files"), "case.task_files"))
        task_files.push_back(ygg::common::resolve_path(std::filesystem::path(BENCHMARKS_DIR), std::string(task_file.as_string())));

    return EquivalenceGraphFigureCase { ygg::common::as_string(object, "name", "case"),
                                        ygg::common::resolve_path(std::filesystem::path(BENCHMARKS_DIR), ygg::common::as_string(object, "domain_file", "case")),
                                        std::move(task_files),
                                        parse_equivalence_policy_mode(ygg::common::as_string(object, "equivalence_policy", "case")),
                                        ygg::common::as_size(object, "expected_num_vertices", "case"),
                                        ygg::common::as_size(object, "expected_num_edges", "case") };
}

std::vector<EquivalenceGraphFigureCase> load_cases()
{
    const auto suite = load_fixture_json("datasets/equivalence_graph.json");
    const auto& suite_object = ygg::common::as_object(suite, "suite");
    const auto* tests_value = suite_object.if_contains("tests");
    if (!tests_value)
        throw std::runtime_error("suite.tests is required.");

    auto result = std::vector<EquivalenceGraphFigureCase> {};
    for (const auto& case_value : ygg::common::as_array(*tests_value, "suite.tests"))
        result.push_back(parse_case(ygg::common::as_object(case_value, "case")));
    return result;
}

}  // namespace

class EquivalenceGraphTest : public ::testing::TestWithParam<EquivalenceGraphFigureCase>
{
};

TEST_P(EquivalenceGraphTest, MatchesExpectedProperties)
{
    const auto& param = GetParam();
    auto contexts = make_ground_contexts(param.domain_file, param.task_files);

    const auto result = datasets::generate_equivalence_graph(contexts, param.equivalence_policy);
    const auto figure = fmt::format("{}", *result.graph);

    std::cout << "EQUIVALENCE_GRAPH_FIGURE " << param.name << "\n" << figure;
    EXPECT_EQ(result.graph->get_forward_graph().get_num_vertices(), param.expected_num_vertices);
    EXPECT_EQ(result.graph->get_forward_graph().get_num_edges(), param.expected_num_edges);
    ASSERT_EQ(result.state_graph_results.size(), param.task_files.size());
    for (const auto& state_graph_result : result.state_graph_results)
    {
        ASSERT_NE(state_graph_result.graph, nullptr);
        EXPECT_GT(state_graph_result.graph->get_forward_graph().get_num_vertices(), 0);
    }
}

TEST(EquivalenceGraphTest, RejectsUnsupportedPolicyMode)
{
    namespace p = tyr::planning;

    auto contexts = datasets::TaskSearchContextList<p::GroundTag> {};

    EXPECT_THROW(
        try {
            static_cast<void>(datasets::generate_equivalence_graph(contexts, static_cast<datasets::EquivalencePolicyMode>(999)));
        } catch (const std::runtime_error& error) {
            EXPECT_STREQ(error.what(), "Unsupported equivalence policy mode: 999.");
            throw;
        },
        std::runtime_error);
}

TEST(EquivalenceGraphTest, RejectsCrossGraphConcreteStateEdge)
{
    const auto root = gripper_benchmark_root();
    auto contexts = make_ground_contexts(root / "domain.pddl", { root / "test-1.pddl", root / "test-1.pddl" });

    EXPECT_THROW(
        try {
            static_cast<void>(datasets::generate_equivalence_graph(contexts, datasets::EquivalencePolicyMode::GI));
        } catch (const std::runtime_error& error) {
            EXPECT_STREQ(error.what(), "Cannot create a concrete state edge to a representative in a different state graph.");
            throw;
        },
        std::runtime_error);
}

INSTANTIATE_TEST_SUITE_P(RunirDatasets,
                         EquivalenceGraphTest,
                         ::testing::ValuesIn(load_cases()),
                         [](const testing::TestParamInfo<EquivalenceGraphFigureCase>& info) { return info.param.name; });

}  // namespace runir::tests
