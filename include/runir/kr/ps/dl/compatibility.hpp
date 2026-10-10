#ifndef RUNIR_KR_PS_DL_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_DL_COMPATIBILITY_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/dl/transition_evaluation_context.hpp"
#include "runir/kr/ps/family_traits.hpp"
#include "runir/kr/dl/semantics/evaluation_context.hpp"

#include <concepts>
#include <tyr/planning/declarations.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps
{

template<tyr::TaskKind Kind,
         runir::kr::FamilyTag Family,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         runir::kr::ps::dl::ConditionObservationTag<FeatureTag> ObservationTag,
         typename C,
         runir::kr::dl::semantics::EvaluationContextConcept<DlFamilyFor<Family>, Kind> Context>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition,
                        Context& context)
{
    const auto value = runir::kr::ps::evaluate<Kind>(condition.get_feature(), context).get();
    if constexpr (std::same_as<FeatureTag, runir::kr::dl::BooleanTag> && std::same_as<ObservationTag, runir::kr::ps::dl::Positive>)
        return value;
    else if constexpr (std::same_as<FeatureTag, runir::kr::dl::BooleanTag> && std::same_as<ObservationTag, runir::kr::ps::dl::Negative>)
        return !value;
    else if constexpr (std::same_as<FeatureTag, runir::kr::dl::NumericalTag> && std::same_as<ObservationTag, runir::kr::ps::dl::EqualZero>)
        return value == 0;
    else if constexpr (std::same_as<FeatureTag, runir::kr::dl::NumericalTag> && std::same_as<ObservationTag, runir::kr::ps::dl::GreaterZero>)
        return value > 0;
}

template<tyr::TaskKind Kind,
         runir::kr::FamilyTag Family,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         runir::kr::ps::dl::ConditionObservationTag<FeatureTag> ObservationTag,
         typename C,
         runir::kr::ps::dl::TransitionEvaluationContextConcept<Family, Kind> Context>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition,
                        Context& context)
{
    return is_compatible_with<Kind>(condition, context.get_source_context());
}

template<tyr::TaskKind Kind,
         runir::kr::FamilyTag Family,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         runir::kr::ps::dl::EffectObservationTag<FeatureTag> ObservationTag,
         typename C,
         runir::kr::ps::dl::TransitionEvaluationContextConcept<Family, Kind> Context>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> effect, Context& context)
{
    const auto target = runir::kr::ps::evaluate<Kind>(effect.get_feature(), context.get_target_context()).get();

    if constexpr (std::same_as<FeatureTag, runir::kr::dl::BooleanTag> && std::same_as<ObservationTag, runir::kr::ps::dl::Positive>)
        return target;
    else if constexpr (std::same_as<FeatureTag, runir::kr::dl::BooleanTag> && std::same_as<ObservationTag, runir::kr::ps::dl::Negative>)
        return !target;
    else
    {
        const auto source = runir::kr::ps::evaluate<Kind>(effect.get_feature(), context.get_source_context()).get();

        if constexpr (std::same_as<ObservationTag, runir::kr::ps::dl::Unchanged>)
            return source == target;
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::NumericalTag> && std::same_as<ObservationTag, runir::kr::ps::dl::Increases>)
            return target > source;
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::NumericalTag> && std::same_as<ObservationTag, runir::kr::ps::dl::Decreases>)
            return target < source;
    }
}

}  // namespace runir::kr::ps

#endif
