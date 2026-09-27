#ifndef RUNIR_KR_PS_DL_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_DL_COMPATIBILITY_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/dl/transition_evaluation_context.hpp"

#include <concepts>
#include <tyr/planning/declarations.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps
{

template<runir::kr::FamilyTag Family, typename FeatureTag, typename ObservationTag, typename C, tyr::TaskKind Kind>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition,
                        runir::kr::dl::semantics::StateEvaluationContext<typename PsFamilyTraits<Family>::DlFamily, Kind>& context)
{
    const auto value = runir::kr::ps::evaluate(condition.get_feature(), context).get();
    if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Positive>)
        return value;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Negative>)
        return !value;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::EqualZero>)
        return value == 0;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::GreaterZero>)
        return value > 0;
}

template<runir::kr::FamilyTag Family, typename FeatureTag, typename ObservationTag, typename C, tyr::TaskKind Kind>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition,
                        runir::kr::ps::dl::TransitionEvaluationContext<Family, Kind>& context)
{
    return is_compatible_with(condition, context.get_source_context());
}

template<runir::kr::FamilyTag Family, typename FeatureTag, typename ObservationTag, typename C, tyr::TaskKind Kind>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> effect,
                        runir::kr::ps::dl::TransitionEvaluationContext<Family, Kind>& context)
{
    const auto target = runir::kr::ps::evaluate(effect.get_feature(), context.get_target_context()).get();

    if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Positive>)
        return target;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Negative>)
        return !target;
    else
    {
        const auto source = runir::kr::ps::evaluate(effect.get_feature(), context.get_source_context()).get();

        if constexpr (std::same_as<ObservationTag, runir::kr::ps::dl::Unchanged>)
            return source == target;
        else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Increases>)
            return target > source;
        else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Decreases>)
            return target < source;
    }
}

}  // namespace runir::kr::ps

#endif
