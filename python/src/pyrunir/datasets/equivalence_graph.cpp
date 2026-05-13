#include "module.hpp"

#include <pyrunir/graphs/graph.hpp>
#include <runir/datasets/equivalence_graph.hpp>
#include <runir/datasets/formatter.hpp>
#include <tyr/common/python/bindings.hpp>
#include <tyr/common/python/type_casters.hpp>

namespace runir::datasets
{

using runir::graphs::bind_bidirectional_graph;
using runir::graphs::bind_bidirectional_static_graph;
using runir::graphs::bind_forward_graph;
using runir::graphs::bind_readable_graph;

void bind_equivalence_graph(nb::module_& m)
{
    auto vertex_label = nb::class_<EquivalenceVertexLabel>(m, "EquivalenceVertexLabel")  //
                            .def(nb::init<>())
                            .def_rw("state_graph_index", &EquivalenceVertexLabel::state_graph_index)
                            .def_rw("state_vertex_index", &EquivalenceVertexLabel::state_vertex_index);
    tyr::add_print(vertex_label);
    tyr::add_hash(vertex_label);

    auto annotated_vertex_label = nb::class_<AnnotatedEquivalenceVertexLabel>(m, "AnnotatedEquivalenceVertexLabel")  //
                                      .def(nb::init<>())
                                      .def_rw("state_graph_index", &AnnotatedEquivalenceVertexLabel::state_graph_index)
                                      .def_rw("state_vertex_index", &AnnotatedEquivalenceVertexLabel::state_vertex_index)
                                      .def_rw("goal_distance", &AnnotatedEquivalenceVertexLabel::goal_distance)
                                      .def_rw("is_initial", &AnnotatedEquivalenceVertexLabel::is_initial)
                                      .def_rw("is_goal", &AnnotatedEquivalenceVertexLabel::is_goal)
                                      .def_rw("is_alive", &AnnotatedEquivalenceVertexLabel::is_alive)
                                      .def_rw("is_unsolvable", &AnnotatedEquivalenceVertexLabel::is_unsolvable);
    tyr::add_print(annotated_vertex_label);
    tyr::add_hash(annotated_vertex_label);

    auto edge_label = nb::class_<EquivalenceEdgeLabel>(m, "EquivalenceEdgeLabel")  //
                          .def(nb::init<>())
                          .def_rw("state_graph_index", &EquivalenceEdgeLabel::state_graph_index)
                          .def_rw("state_edge_index", &EquivalenceEdgeLabel::state_edge_index);
    tyr::add_print(edge_label);
    tyr::add_hash(edge_label);

    auto builder = nb::class_<EquivalenceGraphBuilder>(m, "EquivalenceGraphBuilder");
    builder.def(nb::init<>()).def("clear", &EquivalenceGraphBuilder::clear);
    bind_readable_graph(builder);

    auto static_graph = nb::class_<StaticEquivalenceGraph>(m, "StaticEquivalenceGraph");
    static_graph.def(nb::init<>()).def(nb::init<const EquivalenceGraphBuilder&>());
    bind_readable_graph(static_graph);
    bind_forward_graph(static_graph);

    auto backward_graph = nb::class_<graphs::BackwardStaticGraphView<StaticEquivalenceGraph>>(m, "BackwardEquivalenceGraphView");
    bind_readable_graph(backward_graph);
    bind_forward_graph(backward_graph);

    auto graph = nb::class_<EquivalenceGraph>(m, "EquivalenceGraph");
    graph.def(nb::init<const EquivalenceGraphBuilder&>());
    bind_bidirectional_static_graph(graph);

    auto annotated_builder = nb::class_<AnnotatedEquivalenceGraphBuilder>(m, "AnnotatedEquivalenceGraphBuilder");
    annotated_builder.def(nb::init<>()).def("clear", &AnnotatedEquivalenceGraphBuilder::clear);
    bind_readable_graph(annotated_builder);

    auto static_annotated_graph = nb::class_<StaticAnnotatedEquivalenceGraph>(m, "StaticAnnotatedEquivalenceGraph");
    static_annotated_graph.def(nb::init<>()).def(nb::init<const AnnotatedEquivalenceGraphBuilder&>());
    bind_readable_graph(static_annotated_graph);
    bind_forward_graph(static_annotated_graph);

    auto backward_annotated_graph = nb::class_<graphs::BackwardStaticGraphView<StaticAnnotatedEquivalenceGraph>>(m, "BackwardAnnotatedEquivalenceGraphView");
    bind_readable_graph(backward_annotated_graph);
    bind_forward_graph(backward_annotated_graph);

    auto annotated_graph = nb::class_<AnnotatedEquivalenceGraph>(m, "AnnotatedEquivalenceGraph");
    annotated_graph.def(nb::init<const AnnotatedEquivalenceGraphBuilder&>());
    bind_bidirectional_static_graph(annotated_graph);
}

}  // namespace runir::datasets
