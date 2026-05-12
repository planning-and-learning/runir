#ifndef PYRUNIR_GRAPHS_GRAPH_HPP_
#define PYRUNIR_GRAPHS_GRAPH_HPP_

#include "module.hpp"

#include <functional>
#include <nanobind/make_iterator.h>
#include <nanobind/stl/array.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/string.h>
#include <ranges>
#include <runir/graphs/algorithms.hpp>
#include <runir/graphs/bidirectional_static_graph.hpp>
#include <runir/graphs/dynamic_graph.hpp>
#include <runir/graphs/formatter.hpp>
#include <runir/graphs/static_graph.hpp>
#include <tyr/common/python/bindings.hpp>

using namespace nanobind::literals;

namespace runir::graphs
{

struct PyObjectProperty
{
    nb::object value;

    PyObjectProperty() = default;
    explicit PyObjectProperty(nb::object value_) : value(std::move(value_))
    {
        if (PyObject_Hash(value.ptr()) == -1 && PyErr_Occurred())
            throw nb::python_error();
    }
};

}  // namespace runir::graphs

namespace tyr
{

template<>
struct Hash<runir::graphs::PyObjectProperty>
{
    auto operator()(const runir::graphs::PyObjectProperty& property) const -> std::size_t
    {
        const auto hash = PyObject_Hash(property.value.ptr());
        if (hash == -1 && PyErr_Occurred())
            throw nb::python_error();
        return static_cast<std::size_t>(hash);
    }
};

template<>
struct EqualTo<runir::graphs::PyObjectProperty>
{
    auto operator()(const runir::graphs::PyObjectProperty& lhs, const runir::graphs::PyObjectProperty& rhs) const -> bool
    {
        const auto result = PyObject_RichCompareBool(lhs.value.ptr(), rhs.value.ptr(), Py_EQ);
        if (result == -1)
            throw nb::python_error();
        return result == 1;
    }
};

}  // namespace tyr

namespace fmt
{

template<>
struct formatter<runir::graphs::PyObjectProperty, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::graphs::PyObjectProperty& property, FormatContext& ctx) const
    {
        const auto text = nb::cast<std::string>(nb::repr(property.value));
        return formatter<std::string_view>::format(text, ctx);
    }
};

}  // namespace fmt

namespace runir::graphs
{

using PyObjectDynamicGraph = graphs::DynamicGraph<PyObjectProperty, PyObjectProperty>;
using PyObjectStaticGraphBuilder = graphs::StaticGraphBuilder<PyObjectProperty, PyObjectProperty>;
using PyObjectStaticGraph = graphs::StaticGraph<PyObjectProperty, PyObjectProperty>;
using PyObjectBackwardStaticGraphView = graphs::BackwardStaticGraphView<PyObjectStaticGraph>;
using PyObjectBidirectionalStaticGraph = graphs::BidirectionalStaticGraph<PyObjectProperty, PyObjectProperty>;

inline void bind_graph_certificates(nb::module_& m)
{
    using ColorRefinementCertificate = graphs::color_refinement::Certificate<PyObjectProperty>;
    using WeisfeilerLeman2Signature = graphs::weisfeiler_leman::Signature<2>;
    using WeisfeilerLeman3Signature = graphs::weisfeiler_leman::Signature<3>;
    using WeisfeilerLeman2Certificate = graphs::weisfeiler_leman::Certificate<2>;
    using WeisfeilerLeman3Certificate = graphs::weisfeiler_leman::Certificate<3>;

    auto weisfeiler_leman_2_signature = nb::class_<WeisfeilerLeman2Signature>(m, "WeisfeilerLeman2Signature")  //
                                            .def(nb::init<>())
                                            .def_rw("color", &WeisfeilerLeman2Signature::color)
                                            .def_rw("neighbor_colors", &WeisfeilerLeman2Signature::neighbor_colors);
    tyr::add_print(weisfeiler_leman_2_signature);
    tyr::add_hash(weisfeiler_leman_2_signature);

    auto weisfeiler_leman_3_signature = nb::class_<WeisfeilerLeman3Signature>(m, "WeisfeilerLeman3Signature")  //
                                            .def(nb::init<>())
                                            .def_rw("color", &WeisfeilerLeman3Signature::color)
                                            .def_rw("neighbor_colors", &WeisfeilerLeman3Signature::neighbor_colors);
    tyr::add_print(weisfeiler_leman_3_signature);
    tyr::add_hash(weisfeiler_leman_3_signature);

    auto color_refinement_certificate = nb::class_<ColorRefinementCertificate>(m, "ColorRefinementCertificate")  //
                                            .def(nb::init<>())
                                            .def("get_round_summaries", &ColorRefinementCertificate::get_round_summaries)
                                            .def("get_colors", &ColorRefinementCertificate::get_colors);
    tyr::add_print(color_refinement_certificate);
    tyr::add_hash(color_refinement_certificate);

    auto weisfeiler_leman_2_certificate = nb::class_<WeisfeilerLeman2Certificate>(m, "WeisfeilerLeman2Certificate")  //
                                              .def(nb::init<>())
                                              .def("get_round_summaries", &WeisfeilerLeman2Certificate::get_round_summaries)
                                              .def("get_colors", &WeisfeilerLeman2Certificate::get_colors);
    tyr::add_print(weisfeiler_leman_2_certificate);
    tyr::add_hash(weisfeiler_leman_2_certificate);

    auto weisfeiler_leman_3_certificate = nb::class_<WeisfeilerLeman3Certificate>(m, "WeisfeilerLeman3Certificate")  //
                                              .def(nb::init<>())
                                              .def("get_round_summaries", &WeisfeilerLeman3Certificate::get_round_summaries)
                                              .def("get_colors", &WeisfeilerLeman3Certificate::get_colors);
    tyr::add_print(weisfeiler_leman_3_certificate);
    tyr::add_hash(weisfeiler_leman_3_certificate);
}

template<typename Graph, typename Range>
auto make_graph_iterator(const char* name, Range&& range)
{
    static_assert(std::ranges::borrowed_range<Range>);

    return nb::make_iterator(nb::type<Graph>(), name, std::ranges::begin(range), std::ranges::end(range));
}

template<typename Graph>
auto get_vertex_property(const Graph& graph, graphs::VertexIndex vertex)
{
    return graph.get_vertex(vertex).get_property().value;
}

template<typename Graph>
auto get_edge_property(const Graph& graph, graphs::EdgeIndex edge)
{
    return graph.get_edge(edge).get_property().value;
}

template<typename Graph>
void bind_basic_graph(nb::class_<Graph>& cls)
{
    tyr::add_print(cls);

    cls.def("get_num_vertices", &Graph::get_num_vertices)
        .def("get_num_edges", &Graph::get_num_edges)
        .def(
            "get_vertex_indices",
            [](const Graph& graph) { return make_graph_iterator<Graph>("vertex index iterator", graph.get_vertex_indices()); },
            nb::keep_alive<0, 1>())
        .def(
            "get_edge_indices",
            [](const Graph& graph) { return make_graph_iterator<Graph>("edge index iterator", graph.get_edge_indices()); },
            nb::keep_alive<0, 1>())
        .def("get_source", &Graph::get_source, "edge"_a)
        .def("get_target", &Graph::get_target, "edge"_a)
        .def("get_vertex_property", &get_vertex_property<Graph>, "vertex"_a)
        .def("get_edge_property", &get_edge_property<Graph>, "edge"_a);
}

template<typename Graph>
void bind_forward_graph(nb::class_<Graph>& cls)
{
    cls.def(
           "get_out_edge_indices",
           [](const Graph& graph, graphs::VertexIndex vertex)
           { return make_graph_iterator<Graph>("out edge index iterator", graph.get_out_edge_indices(vertex)); },
           "vertex"_a,
           nb::keep_alive<0, 1>())
        .def("get_out_degree", &Graph::get_out_degree, "vertex"_a);
}

template<typename Graph>
void bind_bidirectional_graph(nb::class_<Graph>& cls)
{
    cls.def(
           "get_in_edge_indices",
           [](const Graph& graph, graphs::VertexIndex vertex)
           { return make_graph_iterator<Graph>("in edge index iterator", graph.get_in_edge_indices(vertex)); },
           "vertex"_a,
           nb::keep_alive<0, 1>())
        .def("get_in_degree", &Graph::get_in_degree, "vertex"_a);
}

template<typename Graph>
void bind_constructible_graph(nb::class_<Graph>& cls)
{
    cls.def(
           "add_vertex",
           [](Graph& graph, nb::object property) { return graph.add_vertex(PyObjectProperty(std::move(property))); },
           "property"_a = nb::none())
        .def(
            "add_directed_edge",
            [](Graph& graph, graphs::VertexIndex source, graphs::VertexIndex target, nb::object property)
            { return graph.add_directed_edge(source, target, PyObjectProperty(std::move(property))); },
            "source"_a,
            "target"_a,
            "property"_a = nb::none())
        .def(
            "add_undirected_edge",
            [](Graph& graph, graphs::VertexIndex lhs, graphs::VertexIndex rhs, nb::object property)
            { return graph.add_undirected_edge(lhs, rhs, PyObjectProperty(std::move(property))); },
            "lhs"_a,
            "rhs"_a,
            "property"_a = nb::none());
}

template<typename Graph>
void bind_graph_algorithms(nb::module_& m)
{
    m.def("strong_components", &graphs::algorithms::strong_components<Graph>, "graph"_a);
    m.def("breadth_first_search", &graphs::algorithms::breadth_first_search<Graph>, "graph"_a, "sources"_a);
    m.def("depth_first_search", &graphs::algorithms::depth_first_search<Graph>, "graph"_a, "sources"_a);
    m.def("topological_sort", &graphs::algorithms::topological_sort<Graph>, "graph"_a);
    m.def("color_refinement_certificate", &graphs::algorithms::color_refinement_certificate<Graph>, "graph"_a);
    m.def("weisfeiler_leman_2_certificate", &graphs::algorithms::weisfeiler_leman_2_certificate<Graph>, "graph"_a);
    m.def("weisfeiler_leman_3_certificate", &graphs::algorithms::weisfeiler_leman_3_certificate<Graph>, "graph"_a);
    m.def("dijkstra_shortest_paths", &graphs::algorithms::dijkstra_shortest_paths<Graph>, "graph"_a, "weights"_a, "sources"_a);
    m.def("floyd_warshall_all_pairs_shortest_paths", &graphs::algorithms::floyd_warshall_all_pairs_shortest_paths<Graph>, "graph"_a, "weights"_a);
}

}  // namespace runir::graphs

#endif
