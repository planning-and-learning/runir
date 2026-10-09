#include "bindings.hpp"

#include <runir/kr/dl/semantics/denotation_caches.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/dl/transition_evaluation_context.hpp>
#include <tyr/planning/state_view.hpp>

namespace runir::kr::ps::base::dl
{

namespace
{

template<tyr::TaskKind Kind>
void bind_transition_evaluation_context(nb::module_& m, const char* name)
{
    using Context = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, Kind>;
    using Storage = runir::kr::dl::semantics::EvaluationStorage<runir::kr::BaseFamilyTag>;

    nb::class_<Context>(m, name)
        .def(nb::new_([](tyr::planning::StateView<Kind> source_state,
                         tyr::planning::StateView<Kind> target_state,
                         runir::kr::dl::semantics::Builder& builder,
                         Storage& source,
                         Storage& target) { return Context(source_state, target_state, builder, source, target); }),
             nb::arg("source_state"),
             nb::arg("target_state"),
             nb::arg("builder"),
             nb::arg("source_storage"),
             nb::arg("target_storage"),
             nb::keep_alive<0, 2>(),
             nb::keep_alive<0, 3>(),
             nb::keep_alive<0, 4>(),
             nb::keep_alive<0, 5>(),
             nb::keep_alive<0, 6>())
        .def("get_source_state", &Context::get_source_state, nb::rv_policy::copy, nb::keep_alive<0, 1>())
        .def("get_target_state", &Context::get_target_state, nb::rv_policy::copy, nb::keep_alive<0, 1>());
}

}  // namespace

void bind_transition_evaluation_contexts(nb::module_& m)
{
    bind_transition_evaluation_context<tyr::GroundTag>(m, "GroundTransitionEvaluationContext");
    bind_transition_evaluation_context<tyr::LiftedTag>(m, "LiftedTransitionEvaluationContext");
}

}  // namespace runir::kr::ps::base::dl
