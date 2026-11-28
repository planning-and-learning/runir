#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <cstddef>
#include <runir/datasets/successors.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/denotation_caches.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/register_values_view.hpp>
#include <runir/kr/ps/dl/transition_evaluation_context.hpp>
#include <tyr/planning/states/state_repository.hpp>

namespace runir::kr::ps::base::dl
{

namespace
{

template<tyr::TaskKind Kind>
void bind_transition_evaluation_context(nb::module_& m, const char* name)
{
    using Context = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, Kind>;
    using Storage = runir::kr::dl::semantics::EvaluationStorage;

    auto cls = nb::class_<Context>(m, name);
    // keep_alive<0, i>: the result (0) keeps constructor argument i alive; self is 1, the arguments start at 2.
    constexpr std::size_t first_argument = 2;
    constexpr std::size_t second_argument = 3;
    constexpr std::size_t third_argument = 4;
    constexpr std::size_t fourth_argument = 5;
    constexpr std::size_t fifth_argument = 6;
    cls.def(
        nb::new_(
            [](const tyr::planning::StateBuilderView<Kind>& source_state,
               const tyr::planning::StateBuilderView<Kind>& target_state,
               runir::kr::dl::semantics::Builder& builder,
               Storage& source,
               Storage& target)
            { return Context(runir::datasets::shared_state<Kind>(source_state), runir::datasets::shared_state<Kind>(target_state), builder, source, target); }),
        nb::arg("source_state"),
        nb::arg("target_state"),
        nb::arg("builder"),
        nb::arg("source_storage"),
        nb::arg("target_storage"),
        nb::keep_alive<0, first_argument>(),
        nb::keep_alive<0, second_argument>(),
        nb::keep_alive<0, third_argument>(),
        nb::keep_alive<0, fourth_argument>(),
        nb::keep_alive<0, fifth_argument>());
    cls.def("get_source_state", &Context::get_source_state, nb::rv_policy::copy, nb::keep_alive<0, 1>())
        .def("get_target_state", &Context::get_target_state, nb::rv_policy::copy, nb::keep_alive<0, 1>());
}

}  // namespace

void bind_transition_evaluation_contexts(nb::module_& m)
{
    bind_transition_evaluation_context<tyr::GroundTag>(m, "GroundTransitionEvaluationContext");
    bind_transition_evaluation_context<tyr::LiftedTag>(m, "LiftedTransitionEvaluationContext");
}

}  // namespace runir::kr::ps::base::dl
