#include "fixtures.hpp"
#include "planning_fixtures.hpp"

#include <gtest/gtest.h>
#include <limits>
#include <runir/datasets/state_graph.hpp>
#include <stdexcept>
#include <tuple>
#include <yggdrasil/containers/associative_containers.hpp>

namespace runir::tests
{

namespace
{

template<tyr::TaskKind Kind>
auto get_transition_counts(const datasets::StateGraph<Kind>& graph)
{
    using Key = std::tuple<tyr::planning::StateView<Kind>, tyr::planning::StateView<Kind>, tyr::formalism::planning::ActionBindingView, ygg::float_t>;
    auto result = ygg::UnorderedMap<Key, size_t> {};
    const auto& forward = graph.get_forward_graph();
    for (const auto edge_index : forward.get_edge_indices())
    {
        const auto& edge = forward.get_edge(edge_index);
        const auto& property = edge.get_property();
        ++result[Key { forward.get_vertex(edge.get_source()).get_property().state,
                       forward.get_vertex(edge.get_target()).get_property().state,
                       property.action,
                       property.cost }];
    }
    return result;
}

template<tyr::TaskKind Kind>
void expect_parallel_state_graph_matches_sequential(const datasets::TaskSearchContextPtr<Kind>& context)
{
    auto sequential_options = datasets::StateGraphGenerationOptions {};
    const auto sequential = datasets::generate_state_graph_result(*context, sequential_options);

    auto parallel_options = sequential_options;
    parallel_options.num_search_workers = 4;
    const auto parallel = datasets::generate_state_graph_result(*context, parallel_options);

    EXPECT_EQ(parallel.status, sequential.status);
    EXPECT_EQ(parallel.graph->get_forward_graph().get_num_vertices(), sequential.graph->get_forward_graph().get_num_vertices());
    EXPECT_EQ(get_transition_counts(*parallel.graph), get_transition_counts(*sequential.graph));
    for (const auto vertex : parallel.graph->get_forward_graph().get_vertex_indices())
        EXPECT_EQ(parallel.graph->get_forward_graph().get_vertex(vertex).get_property().state.get_state_repository(), context->state_repository);
}

}  // namespace

TEST(StateGraphTest, RejectsUnsupportedCostMode)
{
    namespace p = tyr::planning;

    auto context = make_gripper_ground_context();

    auto builder = datasets::StateGraphBuilder<tyr::GroundTag> {};
    const auto initial_state = context->successor_generator->get_initial_node().get_state();
    [[maybe_unused]] const auto initial_vertex = builder.add_vertex(datasets::StateGraphVertexLabel<tyr::GroundTag> { initial_state });
    auto graph = datasets::StateGraph<tyr::GroundTag>(std::move(builder));

    EXPECT_THROW(
        try {
            static_cast<void>(datasets::annotate_state_graph(*context, graph, static_cast<datasets::StateGraphCostMode>(999)));
        } catch (const std::runtime_error& error) {
            EXPECT_STREQ(error.what(), "Unsupported state graph cost mode: 999.");
            throw;
        },
        std::runtime_error);
}

TEST(StateGraphTest, AnnotatesGeneratedGraphWithReachabilityAndGoalMetadata)
{
    auto context = make_gripper_ground_context();
    const auto graph = datasets::generate_state_graph(*context);
    const auto annotated = datasets::annotate_state_graph(*context, *graph, datasets::StateGraphCostMode::UNIT_COST);
    const auto& forward_graph = annotated->get_forward_graph();

    ASSERT_GT(forward_graph.get_num_vertices(), 0);

    auto num_initial = std::size_t { 0 };
    auto num_goal = std::size_t { 0 };
    auto num_alive = std::size_t { 0 };
    for (auto vertex : forward_graph.get_vertex_indices())
    {
        const auto& label = forward_graph.get_vertex(vertex).get_property();
        if (label.is_initial)
        {
            ++num_initial;
            EXPECT_TRUE(label.is_alive);
            EXPECT_FALSE(label.is_unsolvable);
        }
        if (label.is_goal)
        {
            ++num_goal;
            EXPECT_EQ(label.goal_distance, 0);
            EXPECT_TRUE(label.is_alive);
            EXPECT_FALSE(label.is_unsolvable);
        }
        if (label.is_alive)
        {
            ++num_alive;
            EXPECT_FALSE(label.is_unsolvable);
            EXPECT_LT(label.goal_distance, std::numeric_limits<ygg::float_t>::infinity());
        }
        else
        {
            EXPECT_TRUE(label.is_unsolvable);
            EXPECT_EQ(label.goal_distance, std::numeric_limits<ygg::float_t>::infinity());
        }
    }

    EXPECT_EQ(num_initial, 1);
    EXPECT_GT(num_goal, 0);
    EXPECT_GT(num_alive, 0);
}

TEST(StateGraphTest, KeepsInitialVertexWhenStateLimitIsZero)
{
    namespace p = tyr::planning;

    auto context = make_gripper_ground_context();
    auto options = datasets::StateGraphGenerationOptions {};
    options.max_num_states = 0;
    options.num_search_workers = 4;

    const auto result = datasets::generate_state_graph_result(*context, options);

    EXPECT_EQ(result.status, p::SearchStatus::OUT_OF_STATES);
    ASSERT_NE(result.graph, nullptr);
    const auto& graph = result.graph->get_forward_graph();
    ASSERT_EQ(graph.get_num_vertices(), 1);
    EXPECT_EQ(graph.get_vertex(0).get_property().state, context->successor_generator->get_initial_node().get_state());
    EXPECT_EQ(graph.get_vertex(0).get_property().state.get_state_repository(), context->state_repository);
}

TEST(StateGraphTest, RejectsInvalidSearchWorkerCounts)
{
    auto context = make_gripper_ground_context();
    auto options = datasets::StateGraphGenerationOptions {};

    options.num_search_workers = 0;
    EXPECT_THROW(static_cast<void>(datasets::generate_state_graph_result(*context, options)), std::invalid_argument);

    if constexpr (std::numeric_limits<size_t>::max() > std::numeric_limits<ygg::uint_t>::max())
    {
        options.num_search_workers = static_cast<size_t>(std::numeric_limits<ygg::uint_t>::max()) + 1;
        EXPECT_THROW(static_cast<void>(datasets::generate_state_graph_result(*context, options)), std::invalid_argument);
    }
}

TEST(StateGraphTest, ParallelGroundAndLiftedGraphsMatchSequentialGraphs)
{
    const auto root = benchmark_path("classical/tests/gripper");
    expect_parallel_state_graph_matches_sequential(make_ground_context(root / "domain.pddl", root / "test-1.pddl"));
    expect_parallel_state_graph_matches_sequential(make_lifted_context(root / "domain.pddl", root / "test-1.pddl"));
}

}  // namespace runir::tests
