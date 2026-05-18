#include "tyr/common/json_loader.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/json.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <limits>
#include <runir/datasets/serialization.hpp>
#include <runir/datasets/state_graph.hpp>
#include <sstream>
#include <string>
#include <string_view>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/planning.hpp>
#include <vector>

namespace runir::tests
{

namespace
{

struct StateGraphSerializationCase
{
    std::string name;
    std::filesystem::path domain_file;
    std::filesystem::path task_file;
    std::vector<std::string> task_kinds;
    std::vector<std::size_t> thread_counts;
    tyr::uint_t max_num_states = std::numeric_limits<tyr::uint_t>::max();
};

void PrintTo(const StateGraphSerializationCase& test_case, std::ostream* out) { *out << test_case.name; }

template<typename Archive>
std::string serialize_archive(const Archive& archive)
{
    auto out = std::ostringstream {};
    auto boost_archive = boost::archive::text_oarchive(out);
    boost_archive << archive;
    return out.str();
}

template<typename Archive>
Archive round_trip_archive(const Archive& archive)
{
    auto out = std::ostringstream {};
    {
        auto output_archive = boost::archive::text_oarchive(out);
        output_archive << archive;
    }

    auto input = std::istringstream(out.str());
    auto result = Archive {};
    auto input_archive = boost::archive::text_iarchive(input);
    input_archive >> result;
    return result;
}

std::vector<std::string> parse_task_kinds(const boost::json::object& object)
{
    auto result = std::vector<std::string> {};
    for (const auto& value : tyr::common::as_array(*object.if_contains("task_kinds"), "case.task_kinds"))
        result.push_back(std::string(value.as_string()));
    return result;
}

std::vector<std::size_t> parse_thread_counts(const boost::json::object& object)
{
    auto result = std::vector<std::size_t> {};
    for (const auto& value : tyr::common::as_array(*object.if_contains("thread_counts"), "case.thread_counts"))
        result.push_back(static_cast<std::size_t>(value.as_int64()));
    return result;
}

StateGraphSerializationCase parse_case(const boost::json::object& suite, const boost::json::object& object)
{
    return StateGraphSerializationCase { tyr::common::as_string(object, "name", "case"),
                                         tyr::common::suite_path(suite, tyr::common::as_string(object, "domain_file", "case")),
                                         tyr::common::suite_path(suite, tyr::common::as_string(object, "task_file", "case")),
                                         parse_task_kinds(object),
                                         parse_thread_counts(object),
                                         static_cast<tyr::uint_t>(tyr::common::as_size(object, "max_num_states", "case")) };
}

std::vector<StateGraphSerializationCase> load_cases()
{
    const auto suite = tyr::common::load_json_file(tyr::common::root_path() / "tests/unit/datasets/state_graph_serialization.json");
    const auto& suite_object = tyr::common::as_object(suite, "suite");
    const auto* tests_value = suite_object.if_contains("tests");
    if (!tests_value)
        throw std::runtime_error("suite.tests is required.");

    auto result = std::vector<StateGraphSerializationCase> {};
    for (const auto& case_value : tyr::common::as_array(*tests_value, "suite.tests"))
        result.push_back(parse_case(suite_object, tyr::common::as_object(case_value, "case")));
    return result;
}

template<tyr::planning::TaskKind Kind>
void test_state_graph_serialization(tyr::planning::TaskPtr<Kind> task,
                                    tyr::ExecutionContextPtr execution_context,
                                    const std::filesystem::path& task_filepath,
                                    const std::string& problem,
                                    tyr::uint_t max_num_states)
{
    auto context = datasets::TaskSearchContext<Kind>(task, execution_context);

    auto options = datasets::StateGraphGenerationOptions {};
    options.max_num_states = max_num_states;

    const auto result = datasets::generate_state_graph_result(context, options);
    const auto annotated_graph = datasets::annotate_state_graph(context, *result.graph, datasets::StateGraphCostMode::UNIT_COST);
    const auto raw_archive = datasets::serialization::save(task_filepath, *result.graph);
    const auto annotated_archive = datasets::serialization::save(task_filepath, *annotated_graph);
    const auto raw_round_tripped = round_trip_archive(raw_archive);
    const auto annotated_round_tripped = round_trip_archive(annotated_archive);

    EXPECT_EQ(raw_archive.problem, problem);
    EXPECT_EQ(annotated_archive.problem, problem);
    EXPECT_EQ(raw_round_tripped.problem, problem);
    EXPECT_EQ(annotated_round_tripped.problem, problem);
    EXPECT_EQ(serialize_archive(raw_archive), serialize_archive(raw_round_tripped));
    EXPECT_EQ(serialize_archive(annotated_archive), serialize_archive(annotated_round_tripped));

    auto raw_context = datasets::TaskSearchContext<Kind>(task, execution_context);
    const auto deserialized_raw_graph = datasets::serialization::deserialize(raw_context, raw_round_tripped);
    EXPECT_EQ(serialize_archive(raw_round_tripped), serialize_archive(datasets::serialization::save(task_filepath, *deserialized_raw_graph)));

    auto annotated_context = datasets::TaskSearchContext<Kind>(std::move(task), std::move(execution_context));
    const auto deserialized_annotated_graph = datasets::serialization::deserialize(annotated_context, annotated_round_tripped);
    EXPECT_EQ(serialize_archive(annotated_round_tripped), serialize_archive(datasets::serialization::save(task_filepath, *deserialized_annotated_graph)));
}

void test_ground_state_graph_serialization(const tyr::formalism::planning::PlanningTask& planning_task,
                                           tyr::ExecutionContextPtr execution_context,
                                           const std::filesystem::path& task_filepath,
                                           const std::string& problem,
                                           tyr::uint_t max_num_states)
{
    namespace p = tyr::planning;

    auto lifted_task = p::Task<p::LiftedTag>(planning_task);
    auto task = lifted_task.instantiate_ground_task(*execution_context).task;
    test_state_graph_serialization<p::GroundTag>(std::move(task), std::move(execution_context), task_filepath, problem, max_num_states);
}

void test_lifted_state_graph_serialization(tyr::formalism::planning::PlanningTask planning_task,
                                           tyr::ExecutionContextPtr execution_context,
                                           const std::filesystem::path& task_filepath,
                                           const std::string& problem,
                                           tyr::uint_t max_num_states)
{
    namespace p = tyr::planning;

    auto task = p::Task<p::LiftedTag>::create(std::move(planning_task));
    test_state_graph_serialization<p::LiftedTag>(std::move(task), std::move(execution_context), task_filepath, problem, max_num_states);
}

void test_state_graph_serialization(const StateGraphSerializationCase& test_case, std::string_view task_kind, std::size_t num_threads)
{
    namespace fp = tyr::formalism::planning;

    auto parser = fp::Parser(test_case.domain_file);
    const auto planning_task = parser.parse_task(test_case.task_file);
    const auto problem = tyr::common::read_file(test_case.task_file);
    auto execution_context = tyr::ExecutionContext::create(num_threads);

    if (task_kind == "ground")
        return test_ground_state_graph_serialization(planning_task, execution_context, test_case.task_file, problem, test_case.max_num_states);
    if (task_kind == "lifted")
        return test_lifted_state_graph_serialization(planning_task, execution_context, test_case.task_file, problem, test_case.max_num_states);

    throw std::runtime_error(fmt::format("Unsupported task kind: {}", task_kind));
}

}  // namespace

class StateGraphSerializationTest : public ::testing::TestWithParam<StateGraphSerializationCase>
{
};

TEST_P(StateGraphSerializationTest, ArchivesRoundTrip)
{
    const auto& test_case = GetParam();
    ASSERT_FALSE(test_case.task_kinds.empty());
    ASSERT_FALSE(test_case.thread_counts.empty());

    for (const auto& task_kind : test_case.task_kinds)
    {
        for (const auto thread_count : test_case.thread_counts)
        {
            SCOPED_TRACE(fmt::format("{} task_kind={} threads={}", test_case.name, task_kind, thread_count));
            test_state_graph_serialization(test_case, task_kind, thread_count);
        }
    }
}

INSTANTIATE_TEST_SUITE_P(RunirDatasets,
                         StateGraphSerializationTest,
                         ::testing::ValuesIn(load_cases()),
                         [](const testing::TestParamInfo<StateGraphSerializationCase>& info) { return info.param.name; });

}  // namespace runir::tests
