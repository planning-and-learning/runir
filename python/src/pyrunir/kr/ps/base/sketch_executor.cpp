#include "pyrunir/kr/ps/base/module.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>
#include <runir/datasets/state_graph.hpp>
#include <runir/kr/ps/base/sketch_executor.hpp>
#include <tyr/common/python/type_casters.hpp>
#include <tyr/planning/declarations.hpp>

namespace runir::kr::ps::base
{

using namespace nanobind::literals;

namespace
{

template<tyr::planning::TaskKind Kind>
void bind_sketch_proof_results(nb::module_& m, const char* name)
{
    using Results = SketchProofResults<Kind>;

    nb::class_<Results>(m, name)
        .def_ro("status", &Results::status)
        .def_ro("graph", &Results::graph)
        .def_ro("deadend_transitions", &Results::deadend_transitions)
        .def_ro("open_states", &Results::open_states)
        .def_ro("cycle", &Results::cycle)
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
    nb::enum_<SketchProofStatus>(m, "SketchProofStatus")
        .value("SUCCESS", SketchProofStatus::SUCCESS)
        .value("FAILURE", SketchProofStatus::FAILURE)
        .value("OUT_OF_TIME", SketchProofStatus::OUT_OF_TIME)
        .value("OUT_OF_STATES", SketchProofStatus::OUT_OF_STATES);

    bind_sketch_proof_results<tyr::planning::GroundTag>(m, "GroundSketchProofResults");
    bind_sketch_proof_results<tyr::planning::LiftedTag>(m, "LiftedSketchProofResults");
    bind_sketch_search_options<tyr::planning::GroundTag>(m, "GroundSketchSearchOptions");
    bind_sketch_search_options<tyr::planning::LiftedTag>(m, "LiftedSketchSearchOptions");

    m.def("prove_ground_solution",
          [](runir::datasets::TaskSearchContextPtr<tyr::planning::GroundTag> context, SketchView sketch, const SketchSearchOptions<tyr::planning::GroundTag>& options) { return prove_solution(std::move(context), sketch, options); },
          nb::call_guard<nb::gil_scoped_release>(),
          "context"_a,
          "sketch"_a,
          "options"_a = SketchSearchOptions<tyr::planning::GroundTag>());
    m.def("prove_lifted_solution",
          [](runir::datasets::TaskSearchContextPtr<tyr::planning::LiftedTag> context, SketchView sketch, const SketchSearchOptions<tyr::planning::LiftedTag>& options) { return prove_solution(std::move(context), sketch, options); },
          nb::call_guard<nb::gil_scoped_release>(),
          "context"_a,
          "sketch"_a,
          "options"_a = SketchSearchOptions<tyr::planning::LiftedTag>());
    m.def("find_ground_solution",
          [](runir::datasets::TaskSearchContextPtr<tyr::planning::GroundTag> context, SketchView sketch, const SketchSearchOptions<tyr::planning::GroundTag>& options) { return find_solution(std::move(context), sketch, options); },
          nb::call_guard<nb::gil_scoped_release>(),
          "context"_a,
          "sketch"_a,
          "options"_a = SketchSearchOptions<tyr::planning::GroundTag>());
    m.def("find_lifted_solution",
          [](runir::datasets::TaskSearchContextPtr<tyr::planning::LiftedTag> context, SketchView sketch, const SketchSearchOptions<tyr::planning::LiftedTag>& options) { return find_solution(std::move(context), sketch, options); },
          nb::call_guard<nb::gil_scoped_release>(),
          "context"_a,
          "sketch"_a,
          "options"_a = SketchSearchOptions<tyr::planning::LiftedTag>());
}

}  // namespace runir::kr::ps::base
