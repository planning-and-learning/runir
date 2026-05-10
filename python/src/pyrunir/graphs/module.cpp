#include "module.hpp"

#include "graph.hpp"

#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/vector.h>
#include <tyr/common/python/type_casters.hpp>

namespace runir::python
{

void bind_graphs(nb::module_& m)
{
    bind_graph_certificates(m);

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

    bind_graph_algorithms<PyObjectDynamicGraph>(m);
    bind_graph_algorithms<PyObjectStaticGraph>(m);
    bind_graph_algorithms<PyObjectBackwardStaticGraphView>(m);
}

}  // namespace runir::python
