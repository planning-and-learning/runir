#include "bindings.hpp"

#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/base/dl/evaluation_context.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_view.hpp>

namespace runir::kr::ps::base::dl
{

namespace
{

template<tyr::TaskKind Kind>
void bind_evaluation_context(nb::module_& m, const char* name)
{
    using Context = runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, Kind>;

    nb::class_<Context>(m, name)
        .def(nb::init<tyr::planning::StateView<Kind>,
                      tyr::planning::StateView<Kind>,
                      runir::kr::dl::semantics::Builder&,
                      runir::kr::dl::semantics::DenotationRepository&>(),
             nb::arg("source_state"),
             nb::arg("target_state"),
             nb::arg("builder"),
             nb::arg("denotation_repository"),
             nb::keep_alive<1, 4>(),
             nb::keep_alive<1, 5>())
        .def("get_source_state", &Context::get_source_state, nb::rv_policy::copy, nb::keep_alive<0, 1>())
        .def("get_target_state", &Context::get_target_state, nb::rv_policy::copy, nb::keep_alive<0, 1>());
}

}  // namespace

void bind_evaluation_contexts(nb::module_& m)
{
    bind_evaluation_context<tyr::GroundTag>(m, "GroundEvaluationContext");
    bind_evaluation_context<tyr::LiftedTag>(m, "LiftedEvaluationContext");
}

}  // namespace runir::kr::ps::base::dl
