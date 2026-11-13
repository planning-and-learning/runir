#include "module.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/chrono.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/unique_ptr.h>
#include <runir/datasets/formatter.hpp>
#include <runir/datasets/state_graph.hpp>
#include <urd/python/graphs.hpp>
#include <vefr/dot.hpp>
#include <vefr/python/bindings.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::datasets
{

namespace
{

template<tyr::TaskKind Kind>
void bind_state_graph_for_kind(nb::module_& m, const char* class_prefix, const char* function_prefix)
{
    using namespace nb::literals;
    using VertexLabel = StateGraphVertexLabel<Kind>;
    using AnnotatedVertexLabel = AnnotatedStateGraphVertexLabel<Kind>;
    using Graph = StateGraph<Kind>;
    using AnnotatedGraph = AnnotatedStateGraph<Kind>;
    using Result = StateGraphGenerationResult<Kind>;

    auto vertex_label = nb::class_<VertexLabel>(m, (std::string(class_prefix) + "StateGraphVertexLabel").c_str())  //
                            .def_ro("state", &VertexLabel::state, nb::rv_policy::copy, nb::keep_alive<0, 1>());
    ygg::add_print(vertex_label);
    urd::python::add_comparison(vertex_label);
    urd::python::add_hash(vertex_label);

    auto annotated_vertex_label = nb::class_<AnnotatedVertexLabel>(m, (std::string(class_prefix) + "AnnotatedStateGraphVertexLabel").c_str())  //
                                      .def_ro("state", &AnnotatedVertexLabel::state, nb::rv_policy::copy, nb::keep_alive<0, 1>())
                                      .def_ro("goal_distance", &AnnotatedVertexLabel::goal_distance)
                                      .def_ro("is_initial", &AnnotatedVertexLabel::is_initial)
                                      .def_ro("is_goal", &AnnotatedVertexLabel::is_goal)
                                      .def_ro("is_alive", &AnnotatedVertexLabel::is_alive)
                                      .def_ro("is_unsolvable", &AnnotatedVertexLabel::is_unsolvable);
    ygg::add_print(annotated_vertex_label);
    urd::python::add_comparison(annotated_vertex_label);
    urd::python::add_hash(annotated_vertex_label);

    {
        auto graph = nb::class_<Graph>(m, (std::string(class_prefix) + "StateGraph").c_str());
        urd::python::bind_graph(graph);
        vefr::python::bind_algorithms(graph, vefr::vertex_data, vefr::edge_data);
    }
    {
        auto graph = nb::class_<AnnotatedGraph>(m, (std::string(class_prefix) + "AnnotatedStateGraph").c_str());
        urd::python::bind_graph(graph);
        vefr::python::bind_algorithms(graph, vefr::vertex_data, vefr::edge_data);
    }

    nb::class_<Result>(m, (std::string(class_prefix) + "StateGraphGenerationResult").c_str())
        .def_prop_ro(
            "graph",
            [](const Result& self) -> const Graph& { return *self.graph; },
            nb::rv_policy::reference_internal)
        .def_ro("status", &Result::status);

    m.def((std::string("generate_") + function_prefix + "_state_graph").c_str(),
          [](const TaskSearchContextPtr<Kind>& context, const StateGraphGenerationOptions& options) { return generate_state_graph<Kind>(*context, options); },
          "context"_a,
          "options"_a = StateGraphGenerationOptions(),
          nb::keep_alive<0, 1>());
    m.def((std::string("generate_") + function_prefix + "_state_graph_result").c_str(),
          [](const TaskSearchContextPtr<Kind>& context, const StateGraphGenerationOptions& options)
          { return generate_state_graph_result<Kind>(*context, options); },
          "context"_a,
          "options"_a = StateGraphGenerationOptions(),
          nb::keep_alive<0, 1>());
    m.def((std::string("annotate_") + function_prefix + "_state_graph").c_str(),
          [](const TaskSearchContextPtr<Kind>& context, const Graph& state_graph, StateGraphCostMode cost_mode)
          { return annotate_state_graph<Kind>(*context, state_graph, cost_mode); },
          "context"_a,
          "graph"_a,
          "cost_mode"_a,
          nb::keep_alive<0, 1>());
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
    urd::python::add_comparison(edge_label);
    urd::python::add_hash(edge_label);

    bind_state_graph_for_kind<tyr::GroundTag>(m, "Ground", "ground");
    bind_state_graph_for_kind<tyr::LiftedTag>(m, "Lifted", "lifted");
}

}  // namespace runir::datasets
