#ifndef PYRUNIR_GRAPHS_GRAPHS_HPP_
#define PYRUNIR_GRAPHS_GRAPHS_HPP_

#include <functional>
#include <nanobind/make_iterator.h>
#include <nanobind/nanobind.h>
#include <nanobind/stl/pair.h>
#include <ranges>
#include <runir/graphs/bidirectional_static_graph.hpp>
#include <runir/graphs/dynamic_graph.hpp>
#include <runir/graphs/static_graph.hpp>

namespace nb = nanobind;
using namespace nanobind::literals;

namespace runir::python
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

}  // namespace runir::python

namespace tyr
{

template<>
struct Hash<runir::python::PyObjectProperty>
{
    auto operator()(const runir::python::PyObjectProperty& property) const -> std::size_t
    {
        const auto hash = PyObject_Hash(property.value.ptr());
        if (hash == -1 && PyErr_Occurred())
            throw nb::python_error();
        return static_cast<std::size_t>(hash);
    }
};

template<>
struct EqualTo<runir::python::PyObjectProperty>
{
    auto operator()(const runir::python::PyObjectProperty& lhs, const runir::python::PyObjectProperty& rhs) const -> bool
    {
        const auto result = PyObject_RichCompareBool(lhs.value.ptr(), rhs.value.ptr(), Py_EQ);
        if (result == -1)
            throw nb::python_error();
        return result == 1;
    }
};

}  // namespace tyr

namespace runir::python
{

using PyObjectDynamicGraph = graphs::DynamicGraph<PyObjectProperty, PyObjectProperty>;
using PyObjectStaticGraphBuilder = graphs::StaticGraphBuilder<PyObjectProperty, PyObjectProperty>;
using PyObjectStaticGraph = graphs::StaticGraph<PyObjectProperty, PyObjectProperty>;
using PyObjectBackwardStaticGraphView = graphs::BackwardStaticGraphView<PyObjectStaticGraph>;
using PyObjectBidirectionalStaticGraph = graphs::BidirectionalStaticGraph<PyObjectProperty, PyObjectProperty>;

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

void bind_graphs(nb::module_& m);

}  // namespace runir::python

#endif
