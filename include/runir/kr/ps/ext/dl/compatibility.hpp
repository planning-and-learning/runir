#ifndef RUNIR_KR_PS_EXT_DL_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_EXT_DL_COMPATIBILITY_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/dl/transition_evaluation_context.hpp"

#include <concepts>
#include <tyr/planning/declarations.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::ext
{

namespace detail
{

template<typename FeatureTag, typename ObservationTag, typename Value>
bool condition_matches(const Value& value)
{
    if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Positive>)
        return value;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Negative>)
        return !value;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::EqualZero>)
        return value == 0;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::GreaterZero>)
        return value > 0;
}

}  // namespace detail

template<typename FeatureTag, typename ObservationTag, typename C, tyr::TaskKind Kind>
bool is_compatible_with(
    ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition,
    runir::kr::dl::semantics::StateEvaluationContext<runir::kr::ExtFamilyTag, Kind>& context)
{
    const auto value = runir::kr::ps::evaluate(condition.get_feature(), context).get();
    return detail::condition_matches<FeatureTag, ObservationTag>(value);
}

template<typename FeatureTag, typename ObservationTag, typename C, tyr::TaskKind Kind>
bool is_compatible_with(
    ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition,
    runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::ExtFamilyTag, Kind>& context)
{
    return is_compatible_with(condition, context.get_source_context());
}

template<typename FeatureTag, typename ObservationTag, typename C, tyr::TaskKind Kind>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> effect,
                        runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::ExtFamilyTag, Kind>& context)
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

}  // namespace runir::kr::ps::ext

#endif
