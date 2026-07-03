#include "module.hpp"

#include <memory>
#include <nanobind/stl/shared_ptr.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/evaluation_context.hpp>
#include <runir/kr/dl/semantics/ext/evaluation_context.hpp>
#include <tyr/formalism/planning/planning_domain.hpp>
#include <tyr/planning/ground/state_repository.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_repository.hpp>
#include <tyr/planning/lifted/state_view.hpp>

namespace runir::kr::dl::ext
{

namespace
{

template<tyr::planning::TaskKind Kind>
void bind_evaluation_context(nb::module_& m, const char* name)
{
    using Context = runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, Kind>;

    nb::class_<Context>(m, name)
        .def(nb::init<tyr::planning::StateView<Kind>, runir::kr::dl::semantics::Builder&, runir::kr::dl::semantics::DenotationRepository&>(),
             nb::arg("state"),
             nb::arg("builder"),
             nb::arg("denotation_repository"),
             nb::keep_alive<1, 3>(),
             nb::keep_alive<1, 4>())
        .def("get_state", &Context::get_state, nb::rv_policy::reference_internal);
}

}  // namespace

void bind_semantics_repositories(nb::module_& m)
{
    // Builder and the DenotationRepository[Factory] are family-independent and registered once by
    // the base semantics module; the ext ConstructorRepository[Factory] is registered by the ext dl
    // module (kr/dl/ext/repository.cpp). Only the ext-family evaluation contexts are new here.
    bind_evaluation_context<tyr::planning::GroundTag>(m, "GroundEvaluationContext");
    bind_evaluation_context<tyr::planning::LiftedTag>(m, "LiftedEvaluationContext");
}

}  // namespace runir::kr::dl::ext
