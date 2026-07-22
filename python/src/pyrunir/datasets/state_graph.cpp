#include "module.hpp"

#include <nanobind/stl/chrono.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/shared_ptr.h>
#include <pyrunir/graphs/graph.hpp>
#include <runir/datasets/formatter.hpp>
#include <runir/datasets/state_graph.hpp>
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
void bind_state_graph_for_kind(nb::module_& m, const char* class_prefix, const char* function_prefix)
{
    using VertexLabel = StateGraphVertexLabel<Kind>;
    using AnnotatedVertexLabel = AnnotatedStateGraphVertexLabel<Kind>;
    using Builder = StateGraphBuilder<Kind>;
    using StaticGraph = StaticStateGraph<Kind>;
    using BackwardGraph = graphs::BackwardStaticGraphView<StaticGraph>;
    using Graph = StateGraph<Kind>;
    using DynamicGraph = DynamicStateGraph<Kind>;
    using AnnotatedBuilder = AnnotatedStateGraphBuilder<Kind>;
    using StaticAnnotatedGraph = StaticAnnotatedStateGraph<Kind>;
    using BackwardAnnotatedGraph = graphs::BackwardStaticGraphView<StaticAnnotatedGraph>;
    using AnnotatedGraph = AnnotatedStateGraph<Kind>;
    using DynamicAnnotatedGraph = DynamicAnnotatedStateGraph<Kind>;
    using Result = StateGraphGenerationResult<Kind>;

    auto vertex_label = nb::class_<VertexLabel>(m, (std::string(class_prefix) + "StateGraphVertexLabel").c_str())  //
                            .def_ro("state", &VertexLabel::state);
    ygg::add_print(vertex_label);
    ygg::add_comparison(vertex_label);
    ygg::add_hash(vertex_label);

    auto annotated_vertex_label = nb::class_<AnnotatedVertexLabel>(m, (std::string(class_prefix) + "AnnotatedStateGraphVertexLabel").c_str())  //
                                      .def_ro("state", &AnnotatedVertexLabel::state)
                                      .def_ro("goal_distance", &AnnotatedVertexLabel::goal_distance)
                                      .def_ro("is_initial", &AnnotatedVertexLabel::is_initial)
                                      .def_ro("is_goal", &AnnotatedVertexLabel::is_goal)
                                      .def_ro("is_alive", &AnnotatedVertexLabel::is_alive)
                                      .def_ro("is_unsolvable", &AnnotatedVertexLabel::is_unsolvable);
    ygg::add_print(annotated_vertex_label);
    ygg::add_comparison(annotated_vertex_label);
    ygg::add_hash(annotated_vertex_label);

    auto builder = nb::class_<Builder>(m, (std::string(class_prefix) + "StateGraphBuilder").c_str());
    builder.def(nb::init<>()).def("clear", &Builder::clear);
    bind_readable_graph(builder);
    bind_materialized_bidirectional_graph(builder);
    bind_labeled_constructible_graph(builder);

    auto static_graph = nb::class_<StaticGraph>(m, (std::string(class_prefix) + "StaticStateGraph").c_str());
    static_graph.def(nb::init<>()).def(nb::init<const Builder&>());
    bind_readable_graph(static_graph);
    bind_forward_graph(static_graph);

    auto backward_graph = nb::class_<BackwardGraph>(m, (std::string(class_prefix) + "BackwardStateGraphView").c_str());
    bind_readable_graph(backward_graph);
    bind_forward_graph(backward_graph);

    auto graph = nb::class_<Graph>(m, (std::string(class_prefix) + "StateGraph").c_str());
    graph.def(nb::init<const Builder&>());
    bind_bidirectional_static_graph(graph);

    auto dynamic_graph = nb::class_<DynamicGraph>(m, (std::string(class_prefix) + "DynamicStateGraph").c_str());
    bind_labeled_dynamic_graph(dynamic_graph);

    auto annotated_builder = nb::class_<AnnotatedBuilder>(m, (std::string(class_prefix) + "AnnotatedStateGraphBuilder").c_str());
    annotated_builder.def(nb::init<>()).def("clear", &AnnotatedBuilder::clear);
    bind_readable_graph(annotated_builder);
    bind_materialized_bidirectional_graph(annotated_builder);
    bind_labeled_constructible_graph(annotated_builder);

    auto static_annotated_graph = nb::class_<StaticAnnotatedGraph>(m, (std::string(class_prefix) + "StaticAnnotatedStateGraph").c_str());
    static_annotated_graph.def(nb::init<>()).def(nb::init<const AnnotatedBuilder&>());
    bind_readable_graph(static_annotated_graph);
    bind_forward_graph(static_annotated_graph);

    auto backward_annotated_graph = nb::class_<BackwardAnnotatedGraph>(m, (std::string(class_prefix) + "BackwardAnnotatedStateGraphView").c_str());
    bind_readable_graph(backward_annotated_graph);
    bind_forward_graph(backward_annotated_graph);

    auto annotated_graph = nb::class_<AnnotatedGraph>(m, (std::string(class_prefix) + "AnnotatedStateGraph").c_str());
    annotated_graph.def(nb::init<const AnnotatedBuilder&>());
    bind_bidirectional_static_graph(annotated_graph);

    auto dynamic_annotated_graph = nb::class_<DynamicAnnotatedGraph>(m, (std::string(class_prefix) + "DynamicAnnotatedStateGraph").c_str());
    bind_labeled_dynamic_graph(dynamic_annotated_graph);

    nb::class_<Result>(m, (std::string(class_prefix) + "StateGraphGenerationResult").c_str())
        .def_prop_ro(
            "graph",
            [](const Result& self) -> const Graph& { return *self.graph; },
            nb::rv_policy::reference_internal)
        .def_ro("status", &Result::status);

    m.def((std::string("generate_") + function_prefix + "_state_graph").c_str(),
          [](TaskSearchContextPtr<Kind> context, const StateGraphGenerationOptions& options)
          { return std::shared_ptr<Graph>(generate_state_graph<Kind>(*context, options)); },
          "context"_a,
          "options"_a = StateGraphGenerationOptions());
    m.def((std::string("generate_") + function_prefix + "_state_graph_result").c_str(),
          [](TaskSearchContextPtr<Kind> context, const StateGraphGenerationOptions& options)
          { return std::make_shared<Result>(generate_state_graph_result<Kind>(*context, options)); },
          "context"_a,
          "options"_a = StateGraphGenerationOptions());
    m.def((std::string("annotate_") + function_prefix + "_state_graph").c_str(),
          [](TaskSearchContextPtr<Kind> context, const Graph& graph, StateGraphCostMode cost_mode)
          { return std::shared_ptr<AnnotatedGraph>(annotate_state_graph<Kind>(*context, graph, cost_mode)); },
          "context"_a,
          "graph"_a,
          "cost_mode"_a);
}

}  // namespace

void bind_state_graph(nb::module_& m)
{
    nb::class_<StateGraphGenerationOptions>(m, "StateGraphGenerationOptions")
        .def(nb::init<>())
        .def_rw("max_num_states", &StateGraphGenerationOptions::max_num_states)
        .def_rw("max_time", &StateGraphGenerationOptions::max_time);

    auto edge_label = nb::class_<StateGraphEdgeLabel>(m, "StateGraphEdgeLabel")  //
                          .def_ro("action", &StateGraphEdgeLabel::action)
                          .def_ro("cost", &StateGraphEdgeLabel::cost);
    ygg::add_print(edge_label);
    ygg::add_comparison(edge_label);
    ygg::add_hash(edge_label);

    bind_state_graph_for_kind<tyr::planning::GroundTag>(m, "Ground", "ground");
    bind_state_graph_for_kind<tyr::planning::LiftedTag>(m, "Lifted", "lifted");
}

}  // namespace runir::datasets
