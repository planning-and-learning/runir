#include "bindings.hpp"

#include <runir/kr/dl/semantics/denotation_caches.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/base/dl/transition_evaluation_context.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_view.hpp>

namespace runir::kr::ps::base::dl
{

namespace
{

template<tyr::TaskKind Kind>
void bind_transition_evaluation_context(nb::module_& m, const char* name)
{
    using Context = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, Kind>;
    using DenotationCaches = runir::kr::dl::semantics::DenotationCaches<runir::kr::BaseFamilyTag>;

    nb::class_<Context>(m, name)
        .def(
            nb::new_([](tyr::planning::StateView<Kind> source_state,
                        tyr::planning::StateView<Kind> target_state,
                        runir::kr::dl::semantics::Builder& builder,
                        runir::kr::dl::semantics::DenotationRepository& denotation_repository,
                        DenotationCaches& source_caches,
                        DenotationCaches& target_caches)
                     { return Context(source_state, target_state, builder, denotation_repository, builder.get_workspace(), source_caches, target_caches); }),
            nb::arg("source_state"),
            nb::arg("target_state"),
            nb::arg("builder"),
            nb::arg("denotation_repository"),
            nb::arg("source_denotation_caches"),
            nb::arg("target_denotation_caches"),
            nb::keep_alive<0, 4>(),
            nb::keep_alive<0, 5>(),
            nb::keep_alive<0, 6>(),
            nb::keep_alive<0, 7>(),
            nb::keep_alive<6, 4>(),
            nb::keep_alive<6, 5>(),
            nb::keep_alive<7, 4>(),
            nb::keep_alive<7, 5>())
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
