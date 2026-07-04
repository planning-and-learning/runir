#include "pyrunir/kr/ps/base/module.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/vector.h>
#include <pyrunir/graphs/graph.hpp>
#include <runir/datasets/state_graph.hpp>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/sketch_executor.hpp>
#include <runir/kr/ps/base/successor_expander.hpp>
#include <tyr/planning/declarations.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::base
{

using namespace nanobind::literals;
using runir::graphs::bind_forward_graph;
using runir::graphs::bind_readable_graph_methods;

namespace
{
template<tyr::planning::TaskKind Kind>
void bind_sketch_proof_types(nb::module_& m, const char* prefix)
{
    using Graph = SketchProofGraph<Kind>;
    using Results = SketchProofResults<Kind>;

    auto graph = nb::class_<Graph>(m, (std::string(prefix) + "SketchProofGraph").c_str());
    graph.def(nb::init<>());
    bind_readable_graph_methods(graph);
    bind_forward_graph(graph);

    nb::class_<Results>(m, (std::string(prefix) + "SketchProofResults").c_str())
        .def_ro("status", &Results::status)
        .def_ro("graph", &Results::graph, nb::rv_policy::reference_internal)
        .def_ro("deadend_transitions", &Results::deadend_transitions, nb::rv_policy::reference_internal)
        .def_ro("open_states", &Results::open_states, nb::rv_policy::reference_internal)
        .def_ro("cycle", &Results::cycle, nb::rv_policy::reference_internal)
        .def("is_successful", &Results::is_successful);
}

template<tyr::planning::TaskKind Kind>
void bind_sketch_search_options(nb::module_& m, const char* name)
{
    using Options = SketchSearchOptions<Kind>;

    nb::class_<Options>(m, name)
        .def(nb::init<>())
        .def_rw("brfs_options", &Options::brfs_options)
        .def_rw("iw_options", &Options::iw_options)
        .def_rw("siw_options", &Options::siw_options)
        .def_rw("max_arity", &Options::max_arity);
}

}  // namespace

void bind_sketch_executor(nb::module_& m)
{
    auto edge_label = nb::class_<SketchProofEdgeLabel>(m, "SketchProofEdgeLabel")
                          .def_ro("transition", &SketchProofEdgeLabel::transition, nb::rv_policy::reference_internal)
                          .def_ro("rule", &SketchProofEdgeLabel::rule, nb::rv_policy::reference_internal);
    ygg::add_print(edge_label);
    ygg::add_hash(edge_label);

    nb::enum_<SketchProofStatus>(m, "SketchProofStatus")
        .value("SUCCESS", SketchProofStatus::SUCCESS)
        .value("FAILURE", SketchProofStatus::FAILURE)
        .value("OUT_OF_TIME", SketchProofStatus::OUT_OF_TIME)
        .value("OUT_OF_STATES", SketchProofStatus::OUT_OF_STATES);

    bind_sketch_proof_types<tyr::planning::GroundTag>(m, "Ground");
    bind_sketch_proof_types<tyr::planning::LiftedTag>(m, "Lifted");
    bind_sketch_search_options<tyr::planning::GroundTag>(m, "GroundSketchSearchOptions");
    bind_sketch_search_options<tyr::planning::LiftedTag>(m, "LiftedSketchSearchOptions");

    using Kind = tyr::planning::GroundTag;
    using Context = EvaluationContext<Kind>;
    using Expander = SuccessorExpander<Kind>;

    auto execution_context = nb::class_<Context>(m, "ExecutionContext").def_prop_ro("state", &Context::get_state, nb::keep_alive<0, 1>());
    ygg::add_hash(execution_context);

    nb::class_<Expander>(m, "SuccessorExpander")
        .def(nb::init<SketchView>(), "sketch"_a)
        .def("context_at", &Expander::context_at, "state"_a, nb::keep_alive<0, 1>())
        .def("matching_rule", &Expander::matching_rule, "context"_a, "target_state"_a);

    m.def(
        "prove_ground_solution",
        [](runir::datasets::TaskSearchContextPtr<tyr::planning::GroundTag> context,
           SketchView sketch,
           const SketchSearchOptions<tyr::planning::GroundTag>& options) { return prove_solution(std::move(context), sketch, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "sketch"_a,
        "options"_a = SketchSearchOptions<tyr::planning::GroundTag>());
    m.def(
        "prove_lifted_solution",
        [](runir::datasets::TaskSearchContextPtr<tyr::planning::LiftedTag> context,
           SketchView sketch,
           const SketchSearchOptions<tyr::planning::LiftedTag>& options) { return prove_solution(std::move(context), sketch, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "sketch"_a,
        "options"_a = SketchSearchOptions<tyr::planning::LiftedTag>());
    m.def(
        "find_ground_solution",
        [](runir::datasets::TaskSearchContextPtr<tyr::planning::GroundTag> context,
           SketchView sketch,
           const SketchSearchOptions<tyr::planning::GroundTag>& options) { return find_solution(std::move(context), sketch, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "sketch"_a,
        "options"_a = SketchSearchOptions<tyr::planning::GroundTag>());
    m.def(
        "find_lifted_solution",
        [](runir::datasets::TaskSearchContextPtr<tyr::planning::LiftedTag> context,
           SketchView sketch,
           const SketchSearchOptions<tyr::planning::LiftedTag>& options) { return find_solution(std::move(context), sketch, options); },
        nb::call_guard<nb::gil_scoped_release>(),
        "context"_a,
        "sketch"_a,
        "options"_a = SketchSearchOptions<tyr::planning::LiftedTag>());
}

}  // namespace runir::kr::ps::base
