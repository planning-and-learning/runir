#include "module.hpp"

#include <nanobind/stl/vector.h>
#include <runir/datasets/state_graph.hpp>
#include <runir/kr/gp/policy_executor.hpp>
#include <tyr/common/python/type_casters.hpp>
#include <tyr/planning/declarations.hpp>

namespace runir::kr::gp
{

using namespace nanobind::literals;

namespace
{

template<tyr::planning::TaskKind Kind>
void bind_policy_proof_results(nb::module_& m, const char* name)
{
    using Results = PolicyProofResults<Kind>;

    nb::class_<Results>(m, name)
        .def_ro("status", &Results::status)
        .def_ro("graph", &Results::graph)
        .def_ro("deadend_transitions", &Results::deadend_transitions)
        .def_ro("open_states", &Results::open_states)
        .def_ro("cycle", &Results::cycle)
        .def("is_successful", &Results::is_successful);
}

template<tyr::planning::TaskKind Kind>
void bind_policy_search_options(nb::module_& m, const char* name)
{
    using Options = PolicySearchOptions<Kind>;

    nb::class_<Options>(m, name)
        .def(nb::init<>())
        .def_rw("brfs_options", &Options::brfs_options)
        .def_rw("iw_options", &Options::iw_options)
        .def_rw("siw_options", &Options::siw_options)
        .def_rw("max_arity", &Options::max_arity);
}

}  // namespace

void bind_policy_executor(nb::module_& m)
{
    nb::enum_<PolicyProofStatus>(m, "PolicyProofStatus").value("SUCCESS", PolicyProofStatus::SUCCESS).value("FAILURE", PolicyProofStatus::FAILURE);

    bind_policy_proof_results<tyr::planning::GroundTag>(m, "GroundPolicyProofResults");
    bind_policy_proof_results<tyr::planning::LiftedTag>(m, "LiftedPolicyProofResults");
    bind_policy_search_options<tyr::planning::GroundTag>(m, "GroundPolicySearchOptions");
    bind_policy_search_options<tyr::planning::LiftedTag>(m, "LiftedPolicySearchOptions");

    m.def("prove_ground_solution", &prove_solution<tyr::planning::GroundTag>, "graph"_a, "policy"_a);
    m.def("prove_lifted_solution", &prove_solution<tyr::planning::LiftedTag>, "graph"_a, "policy"_a);
    m.def("find_ground_solution",
          &find_solution<tyr::planning::GroundTag>,
          nb::call_guard<nb::gil_scoped_release>(),
          "context"_a,
          "policy"_a,
          "options"_a = PolicySearchOptions<tyr::planning::GroundTag>());
    m.def("find_lifted_solution",
          &find_solution<tyr::planning::LiftedTag>,
          nb::call_guard<nb::gil_scoped_release>(),
          "context"_a,
          "policy"_a,
          "options"_a = PolicySearchOptions<tyr::planning::LiftedTag>());
}

}  // namespace runir::kr::gp
