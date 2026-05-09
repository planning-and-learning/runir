#include "graphs.hpp"

#include <nanobind/stl/vector.h>
#include <runir/graphs/bgl/algorithms.hpp>

namespace runir::python
{

namespace
{

template<typename T>
auto to_list(const std::vector<T>& data)
{
    auto result = nb::list();
    for (const auto& value : data)
        result.append(value);
    return result;
}

template<typename K, typename V>
auto to_dict(const tyr::UnorderedMap<K, V>& data)
{
    auto result = nb::dict();
    for (const auto& [key, value] : data)
        result[nb::cast(key)] = nb::cast(value);
    return result;
}

template<typename T>
auto to_python(const std::vector<T>& data)
{
    return to_list(data);
}

template<typename K, typename V>
auto to_python(const tyr::UnorderedMap<K, V>& data)
{
    return to_dict(data);
}

template<typename T>
auto to_python(const std::vector<std::vector<T>>& data)
{
    auto result = nb::list();
    for (const auto& row : data)
        result.append(to_list(row));
    return result;
}

template<typename K, typename V>
auto to_python(const tyr::UnorderedMap<K, tyr::UnorderedMap<K, V>>& data)
{
    auto result = nb::dict();
    for (const auto& [key, row] : data)
        result[nb::cast(key)] = to_dict(row);
    return result;
}

auto to_weight_map(const nb::dict& weights)
{
    auto result = tyr::UnorderedMap<graphs::EdgeIndex, double>();
    for (auto item : weights)
        result.emplace(nb::cast<graphs::EdgeIndex>(item.first), nb::cast<double>(item.second));
    return result;
}

template<typename Graph>
auto strong_components(const Graph& graph)
{
    const auto [count, components] = graphs::bgl::strong_components(graph);
    return nb::make_tuple(count, to_python(components));
}

template<typename Graph>
auto breadth_first_search(const Graph& graph, const std::vector<graphs::VertexIndex>& sources)
{
    const auto [predecessors, distances] = graphs::bgl::breadth_first_search(graph, sources.begin(), sources.end());
    return nb::make_tuple(to_python(predecessors), to_python(distances));
}

template<typename Graph>
auto depth_first_search(const Graph& graph, const std::vector<graphs::VertexIndex>& sources)
{
    return to_python(graphs::bgl::depth_first_search(graph, sources.begin(), sources.end()));
}

template<typename Graph>
auto topological_sort(const Graph& graph)
{
    return to_python(graphs::bgl::topological_sort(graph));
}

template<typename Graph>
auto dense_dijkstra_shortest_paths(const Graph& graph, const std::vector<double>& weights, const std::vector<graphs::VertexIndex>& sources)
{
    const auto [predecessors, distances] = graphs::bgl::dijkstra_shortest_paths(graph, weights, sources.begin(), sources.end());
    return nb::make_tuple(to_python(predecessors), to_python(distances));
}

template<typename Graph>
auto sparse_dijkstra_shortest_paths(const Graph& graph, const nb::dict& weights, const std::vector<graphs::VertexIndex>& sources)
{
    const auto weight_map = to_weight_map(weights);
    const auto [predecessors, distances] = graphs::bgl::dijkstra_shortest_paths(graph, weight_map, sources.begin(), sources.end());
    return nb::make_tuple(to_python(predecessors), to_python(distances));
}

template<typename Graph>
auto dense_floyd_warshall_all_pairs_shortest_paths(const Graph& graph, const std::vector<double>& weights)
{
    return to_python(graphs::bgl::floyd_warshall_all_pairs_shortest_paths(graph, weights));
}

template<typename Graph>
auto sparse_floyd_warshall_all_pairs_shortest_paths(const Graph& graph, const nb::dict& weights)
{
    return to_python(graphs::bgl::floyd_warshall_all_pairs_shortest_paths(graph, to_weight_map(weights)));
}

template<typename Graph>
void bind_dense_graph_algorithms(nb::module_& m)
{
    m.def("strong_components", &strong_components<Graph>, "graph"_a);
    m.def("breadth_first_search", &breadth_first_search<Graph>, "graph"_a, "sources"_a);
    m.def("depth_first_search", &depth_first_search<Graph>, "graph"_a, "sources"_a);
    m.def("topological_sort", &topological_sort<Graph>, "graph"_a);
    m.def("dijkstra_shortest_paths", &dense_dijkstra_shortest_paths<Graph>, "graph"_a, "weights"_a, "sources"_a);
    m.def("floyd_warshall_all_pairs_shortest_paths", &dense_floyd_warshall_all_pairs_shortest_paths<Graph>, "graph"_a, "weights"_a);
}

template<typename Graph>
void bind_sparse_graph_algorithms(nb::module_& m)
{
    m.def("strong_components", &strong_components<Graph>, "graph"_a);
    m.def("breadth_first_search", &breadth_first_search<Graph>, "graph"_a, "sources"_a);
    m.def("depth_first_search", &depth_first_search<Graph>, "graph"_a, "sources"_a);
    m.def("topological_sort", &topological_sort<Graph>, "graph"_a);
    m.def("dijkstra_shortest_paths", &sparse_dijkstra_shortest_paths<Graph>, "graph"_a, "weights"_a, "sources"_a);
    m.def("floyd_warshall_all_pairs_shortest_paths", &sparse_floyd_warshall_all_pairs_shortest_paths<Graph>, "graph"_a, "weights"_a);
}

}  // namespace

void bind_graphs(nb::module_& m)
{
    auto dynamic_graph = nb::class_<PyObjectDynamicGraph>(m, "DynamicGraph");
    dynamic_graph.def(nb::init<>()).def("clear", &PyObjectDynamicGraph::clear);
    bind_basic_graph(dynamic_graph);
    bind_forward_graph(dynamic_graph);
    bind_bidirectional_graph(dynamic_graph);
    bind_constructible_graph(dynamic_graph);
    dynamic_graph.def("remove_vertex", &PyObjectDynamicGraph::remove_vertex, "vertex"_a)
        .def("remove_edge", &PyObjectDynamicGraph::remove_edge, "edge"_a)
        .def("contains_vertex", &PyObjectDynamicGraph::contains_vertex, "vertex"_a)
        .def("contains_edge", &PyObjectDynamicGraph::contains_edge, "edge"_a);

    auto static_graph_builder = nb::class_<PyObjectStaticGraphBuilder>(m, "StaticGraphBuilder");
    static_graph_builder.def(nb::init<>()).def("clear", &PyObjectStaticGraphBuilder::clear);
    bind_basic_graph(static_graph_builder);
    bind_constructible_graph(static_graph_builder);

    auto static_graph = nb::class_<PyObjectStaticGraph>(m, "StaticGraph");
    static_graph.def(nb::init<>()).def(nb::init<const PyObjectStaticGraphBuilder&>());
    bind_basic_graph(static_graph);
    bind_forward_graph(static_graph);

    auto backward_static_graph_view = nb::class_<PyObjectBackwardStaticGraphView>(m, "BackwardStaticGraphView");
    bind_basic_graph(backward_static_graph_view);
    bind_forward_graph(backward_static_graph_view);

    auto bidirectional_static_graph = nb::class_<PyObjectBidirectionalStaticGraph>(m, "BidirectionalStaticGraph");
    bidirectional_static_graph.def(nb::init<>())
        .def(nb::init<const PyObjectStaticGraphBuilder&>())
        .def("get_forward_graph", &PyObjectBidirectionalStaticGraph::get_forward_graph, nb::rv_policy::reference_internal)
        .def("get_backward_graph", &PyObjectBidirectionalStaticGraph::get_backward_graph, nb::rv_policy::reference_internal);

    bind_sparse_graph_algorithms<PyObjectDynamicGraph>(m);
    bind_dense_graph_algorithms<PyObjectStaticGraph>(m);
    bind_dense_graph_algorithms<PyObjectBackwardStaticGraphView>(m);
}

}  // namespace runir::python
