#include "bindings.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/dl/semantics/denotation_caches.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <runir/kr/dl/semantics/uns/state_evaluation_context.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_view.hpp>

namespace runir::kr::dl::uns
{

namespace
{

template<tyr::TaskKind Kind>
void bind_state_evaluation_context(nb::module_& m, const char* name)
{
    using Context = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::UnsFamilyTag, Kind>;
    using DenotationCaches = runir::kr::dl::semantics::DenotationCaches<runir::kr::UnsFamilyTag>;
    nb::class_<Context>(m, name)
        .def(
            nb::new_([](tyr::planning::StateView<Kind> state,
                        runir::kr::dl::semantics::Builder& builder,
                        runir::kr::dl::semantics::DenotationRepository& denotation_repository,
                        DenotationCaches& caches)
                     { return Context(state, builder, denotation_repository, builder.get_workspace(), caches); }),
            nb::arg("state"),
            nb::arg("builder"),
            nb::arg("denotation_repository"),
            nb::arg("denotation_caches"),
            nb::keep_alive<0, 3>(),
            nb::keep_alive<0, 4>(),
            nb::keep_alive<0, 5>(),
            nb::keep_alive<5, 3>(),
            nb::keep_alive<5, 4>())
        .def("get_state", &Context::get_state, nb::rv_policy::copy, nb::keep_alive<0, 1>());
}

}  // namespace

void bind_semantics_repositories(nb::module_& m)
{
    using DenotationCaches = runir::kr::dl::semantics::DenotationCaches<runir::kr::UnsFamilyTag>;
    nb::class_<DenotationCaches>(m, "DenotationCaches", "Own evaluated denotations until their cache partition is cleared.")
        .def(nb::init<semantics::DenotationRepository&>(), nb::arg("denotation_repository"))
        .def("clear", nb::overload_cast<>(&DenotationCaches::clear))
        .def("clear", nb::overload_cast<bool>(&DenotationCaches::clear), nb::arg("is_static"));

    bind_state_evaluation_context<tyr::GroundTag>(m, "GroundStateEvaluationContext");
    bind_state_evaluation_context<tyr::LiftedTag>(m, "LiftedStateEvaluationContext");
}

}  // namespace runir::kr::dl::uns
