#include "module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>
#include <pyrunir/graphs/graph.hpp>
#include <runir/datasets/equivalence_graph.hpp>
#include <runir/datasets/formatter.hpp>
#include <stdexcept>
#include <string>
#include <utility>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::datasets
{

using namespace nanobind::literals;
using runir::graphs::bind_bidirectional_graph;
using runir::graphs::bind_bidirectional_static_graph;
using runir::graphs::bind_forward_graph;
using runir::graphs::bind_labeled_constructible_graph;
using runir::graphs::bind_labeled_dynamic_graph;
using runir::graphs::bind_materialized_bidirectional_graph;
using runir::graphs::bind_readable_graph;

namespace
{

template<tyr::planning::TaskKind Kind>
void bind_equivalence_graph_generation_for_kind(nb::module_& m, const char* class_prefix, const char* function_prefix)
{
    using Result = EquivalenceGraphConstructionResult<Kind>;
    using ContextList = TaskSearchContextList<Kind>;

    nb::class_<Result>(m, (std::string(class_prefix) + "EquivalenceGraphConstructionResult").c_str())
        .def_prop_ro(
            "graph",
            [](const Result& self) -> const EquivalenceGraph& { return *self.graph; },
            nb::rv_policy::reference_internal)
        .def("get_num_state_graph_results", [](const Result& self) { return self.state_graph_results.size(); })
        .def(
            "get_state_graph_result",
            [](const Result& self, std::size_t index) -> const StateGraphGenerationResult<Kind>&
            {
                if (index >= self.state_graph_results.size())
                    throw std::out_of_range("State graph result index is out of range.");
                return self.state_graph_results[index];
            },
            "index"_a,
            nb::rv_policy::reference_internal);

    m.def((std::string("generate_") + function_prefix + "_equivalence_graph").c_str(),
          [](ContextList contexts, const EquivalenceGraphGenerationOptions& options)
          { return std::make_shared<Result>(generate_equivalence_graph<Kind>(contexts, options)); },
          "contexts"_a,
          "options"_a = EquivalenceGraphGenerationOptions());
}

}  // namespace

void bind_equivalence_graph(nb::module_& m)
{
    nb::class_<EquivalenceGraphGenerationOptions>(m, "EquivalenceGraphGenerationOptions")
        .def(nb::init<>())
        .def_rw("state_graph_options", &EquivalenceGraphGenerationOptions::state_graph_options)
        .def_rw("policy_mode", &EquivalenceGraphGenerationOptions::policy_mode);

    auto vertex_label = nb::class_<EquivalenceVertexLabel>(m, "EquivalenceVertexLabel")  //
                            .def(nb::init<>())
                            .def_rw("state_graph_index", &EquivalenceVertexLabel::state_graph_index)
                            .def_rw("state_vertex_index", &EquivalenceVertexLabel::state_vertex_index);
    ygg::add_print(vertex_label);
    ygg::add_comparison(vertex_label);
    ygg::add_hash(vertex_label);

    auto annotated_vertex_label = nb::class_<AnnotatedEquivalenceVertexLabel>(m, "AnnotatedEquivalenceVertexLabel")  //
                                      .def(nb::init<>())
                                      .def_rw("state_graph_index", &AnnotatedEquivalenceVertexLabel::state_graph_index)
                                      .def_rw("state_vertex_index", &AnnotatedEquivalenceVertexLabel::state_vertex_index)
                                      .def_rw("goal_distance", &AnnotatedEquivalenceVertexLabel::goal_distance)
                                      .def_rw("is_initial", &AnnotatedEquivalenceVertexLabel::is_initial)
                                      .def_rw("is_goal", &AnnotatedEquivalenceVertexLabel::is_goal)
                                      .def_rw("is_alive", &AnnotatedEquivalenceVertexLabel::is_alive)
                                      .def_rw("is_unsolvable", &AnnotatedEquivalenceVertexLabel::is_unsolvable);
    ygg::add_print(annotated_vertex_label);
    ygg::add_comparison(annotated_vertex_label);
    ygg::add_hash(annotated_vertex_label);

    auto edge_label = nb::class_<EquivalenceEdgeLabel>(m, "EquivalenceEdgeLabel")  //
                          .def(nb::init<>())
                          .def_rw("state_graph_index", &EquivalenceEdgeLabel::state_graph_index)
                          .def_rw("state_edge_index", &EquivalenceEdgeLabel::state_edge_index);
    ygg::add_print(edge_label);
    ygg::add_comparison(edge_label);
    ygg::add_hash(edge_label);

    auto builder = nb::class_<EquivalenceGraphBuilder>(m, "EquivalenceGraphBuilder");
    builder.def(nb::init<>()).def("clear", &EquivalenceGraphBuilder::clear);
    bind_readable_graph(builder);
    bind_materialized_bidirectional_graph(builder);
    bind_labeled_constructible_graph(builder);

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

    auto dynamic_graph = nb::class_<DynamicEquivalenceGraph>(m, "DynamicEquivalenceGraph");
    bind_labeled_dynamic_graph(dynamic_graph);

    auto annotated_builder = nb::class_<AnnotatedEquivalenceGraphBuilder>(m, "AnnotatedEquivalenceGraphBuilder");
    annotated_builder.def(nb::init<>()).def("clear", &AnnotatedEquivalenceGraphBuilder::clear);
    bind_readable_graph(annotated_builder);
    bind_materialized_bidirectional_graph(annotated_builder);
    bind_labeled_constructible_graph(annotated_builder);

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

    auto dynamic_annotated_graph = nb::class_<DynamicAnnotatedEquivalenceGraph>(m, "DynamicAnnotatedEquivalenceGraph");
    bind_labeled_dynamic_graph(dynamic_annotated_graph);

    bind_equivalence_graph_generation_for_kind<tyr::planning::GroundTag>(m, "Ground", "ground");
    bind_equivalence_graph_generation_for_kind<tyr::planning::LiftedTag>(m, "Lifted", "lifted");
}

}  // namespace runir::datasets
