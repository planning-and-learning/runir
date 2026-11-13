#include "pyrunir/kr/ps/base/bindings.hpp"
#include "pyrunir/module.hpp"

#include <functional>
#include <nanobind/stl/chrono.h>
#include <nanobind/stl/function.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/sketch_executor.hpp>
#include <runir/kr/ps/base/successor_expander.hpp>
#include <tyr/planning/declarations.hpp>
#include <urd/python/graphs.hpp>
#include <vefr/dot.hpp>
#include <vefr/python/bindings.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::base
{

namespace
{
template<tyr::TaskKind Kind>
void bind_sketch_proof_types(nb::module_& m, const char* prefix)
{
    using Graph = SketchProofGraph<Kind>;
    using Results = SketchProofResults<Kind>;
    using VertexLabel = SketchProofVertexLabel<Kind>;

    auto vertex_label = nb::class_<VertexLabel>(m, (std::string(prefix) + "SketchProofVertexLabel").c_str())
                            .def_ro("state", &VertexLabel::state, nb::rv_policy::copy, nb::keep_alive<0, 1>())
                            .def_ro("is_initial", &VertexLabel::is_initial)
                            .def_ro("is_goal", &VertexLabel::is_goal)
                            .def_ro("is_alive", &VertexLabel::is_alive)
                            .def_ro("is_unsolvable", &VertexLabel::is_unsolvable);
    ygg::add_print(vertex_label);
    urd::python::add_comparison(vertex_label);
    urd::python::add_hash(vertex_label);

    {
        auto graph = nb::class_<Graph>(m, (std::string(prefix) + "SketchProofGraph").c_str());
        urd::python::bind_graph(graph);
        vefr::python::bind_algorithms(graph, vefr::vertex_data, vefr::edge_data);
    }

    nb::class_<Results>(m, (std::string(prefix) + "SketchProofResults").c_str())
        .def_ro("status", &Results::status)
        .def_ro("graph", &Results::graph, nb::keep_alive<0, 1>())
        .def_ro("plan", &Results::plan, nb::rv_policy::copy, nb::keep_alive<0, 1>())
        .def_ro("deadend_states", &Results::deadend_states)
        .def_ro("open_states", &Results::open_states)
        .def_ro("cycle", &Results::cycle)
        .def_ro("statistics", &Results::statistics)
        .def("is_successful", &Results::is_successful);
}

template<tyr::TaskKind Kind>
void bind_sketch_search_options(nb::module_& m, const char* name)
{
    using Options = SketchSearchOptions<Kind>;

    nb::class_<Options>(m, name)
        .def(nb::init<>())
        .def_rw("universal", &Options::universal)
        .def_rw("classifier", &Options::classifier, nb::for_setter(nb::keep_alive<1, 2>()))
        .def_rw("max_num_states", &Options::max_num_states)
        .def_rw("max_time", &Options::max_time);
}

}  // namespace

void bind_sketch_executor(nb::module_& m)
{
    using namespace nb::literals;
    nb::class_<SketchSearchStatistics>(m, "SketchSearchStatistics")
        .def(nb::init<>())
        .def_ro("num_expanded", &SketchSearchStatistics::num_expanded)
        .def_ro("num_generated", &SketchSearchStatistics::num_generated);

    auto edge_label =
        nb::class_<SketchProofEdgeLabel>(m, "SketchProofEdgeLabel").def_ro("action", &SketchProofEdgeLabel::action).def_ro("rule", &SketchProofEdgeLabel::rule);
    ygg::add_print(edge_label);
    urd::python::add_comparison(edge_label);
    urd::python::add_hash(edge_label);

    bind_sketch_proof_types<tyr::GroundTag>(m, "Ground");
    bind_sketch_proof_types<tyr::LiftedTag>(m, "Lifted");
    bind_sketch_search_options<tyr::GroundTag>(m, "GroundSketchSearchOptions");
    bind_sketch_search_options<tyr::LiftedTag>(m, "LiftedSketchSearchOptions");

    using Kind = tyr::GroundTag;
    using Expander = SuccessorExpander<Kind>;

    nb::class_<Expander>(m, "SuccessorExpander")
        .def(nb::init<runir::kr::TaskContext<Kind>&, SketchView>(), "task_context"_a, "sketch"_a, nb::keep_alive<1, 2>(), nb::keep_alive<1, 3>())
        .def(
            "for_each_successor",
            [](Expander& self,
               tyr::planning::StateView<Kind> state,
               SketchSearchStatistics& statistics,
               const nb::typed<nb::callable, bool(tyr::planning::LabeledNode<Kind>, RuleView)>& emit,
               const std::function<bool()>& stop)
            {
                // The successors borrow the expander's state repository: each keeps the expander alive.
                const auto owner = nb::cast(&self, nb::rv_policy::reference);
                return self.for_each_successor(
                    state,
                    statistics,
                    [&](tyr::planning::LabeledNode<Kind> node, RuleView rule)
                    { return nb::cast<bool>(emit(nb::cast(std::move(node), nb::rv_policy::move, owner), rule)); },
                    stop);
            },
            "state"_a,
            "statistics"_a,
            "emit"_a,
            "stop"_a)
        .def("matching_rule", &Expander::matching_rule, "source_state"_a, "target_state"_a);

    m.def(
        "find_ground_solution",
        [](const runir::kr::TaskContextPtr<tyr::GroundTag>& task_context, SketchView sketch, const SketchSearchOptions<tyr::GroundTag>& options)
        { return find_solution(task_context, sketch, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        nb::keep_alive<0, 2>(),
        "task_context"_a,
        "sketch"_a,
        "options"_a);
    m.def(
        "find_lifted_solution",
        [](const runir::kr::TaskContextPtr<tyr::LiftedTag>& task_context, SketchView sketch, const SketchSearchOptions<tyr::LiftedTag>& options)
        { return find_solution(task_context, sketch, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        nb::keep_alive<0, 2>(),
        "task_context"_a,
        "sketch"_a,
        "options"_a);
}

}  // namespace runir::kr::ps::base
