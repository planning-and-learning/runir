#ifndef RUNIR_KR_PS_BASE_DL_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_BASE_DL_COMPATIBILITY_HPP_

#include "runir/kr/ps/base/dl/condition_view.hpp"
#include "runir/kr/ps/base/dl/effect_view.hpp"
#include "runir/kr/ps/base/dl/evaluation.hpp"
#include "runir/kr/ps/base/dl/evaluation_context.hpp"

#include <concepts>
#include <tyr/planning/declarations.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps
{

template<typename FeatureTag, typename ObservationTag, typename C, tyr::planning::TaskKind Kind>
bool is_compatible_with(ygg::View<ygg::Index<ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition,
                        runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, Kind>& context)
{
    const auto value = evaluate(condition.get_feature(), context.get_source_context());

    if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Positive>)
        return value;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Negative>)
        return !value;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::EqualZero>)
        return value == 0;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::GreaterZero>)
        return value > 0;
}

template<typename FeatureTag, typename ObservationTag, typename C, tyr::planning::TaskKind Kind>
bool is_compatible_with(ygg::View<ygg::Index<ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> effect,
                        runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, Kind>& context)
{
    const auto target = evaluate(effect.get_feature(), context.get_target_context());

    if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Positive>)
        return target;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Negative>)
        return !target;
    else
    {
        const auto source = evaluate(effect.get_feature(), context.get_source_context());

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
