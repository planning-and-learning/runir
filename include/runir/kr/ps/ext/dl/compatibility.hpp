#ifndef RUNIR_KR_PS_EXT_DL_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_EXT_DL_COMPATIBILITY_HPP_

#include "runir/kr/ps/ext/dl/condition_view.hpp"
#include "runir/kr/ps/ext/dl/effect_view.hpp"
#include "runir/kr/ps/ext/dl/evaluation.hpp"

#include <concepts>
#include <tyr/common/types.hpp>
#include <tyr/planning/declarations.hpp>

namespace runir::kr::ps::ext
{

namespace detail
{

template<typename FeatureTag, typename Value>
auto observation_count(const Value& value)
{
    if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
        return value.count();
    else
        return value;
}

}  // namespace detail

template<typename FeatureTag, typename ObservationTag, typename C, tyr::planning::TaskKind Kind>
bool is_compatible_with(tyr::View<tyr::Index<ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition,
                        runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context)
{
    const auto value = evaluate(condition.get_feature(), context);

    if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Positive>)
        return value;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Negative>)
        return !value;
    else if constexpr ((std::same_as<FeatureTag, runir::kr::dl::ConceptTag> || std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
                       && std::same_as<ObservationTag, runir::kr::ps::dl::EqualZero>)
        return detail::observation_count<FeatureTag>(value) == 0;
    else if constexpr ((std::same_as<FeatureTag, runir::kr::dl::ConceptTag> || std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
                       && std::same_as<ObservationTag, runir::kr::ps::dl::GreaterZero>)
        return detail::observation_count<FeatureTag>(value) > 0;
}

template<typename FeatureTag, typename ObservationTag, typename C, tyr::planning::TaskKind Kind>
bool is_compatible_with(tyr::View<tyr::Index<ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition,
                        runir::kr::ps::dl::EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context)
{
    return is_compatible_with(condition, context.get_source_context());
}

template<typename FeatureTag, typename ObservationTag, typename C, tyr::planning::TaskKind Kind>
bool is_compatible_with(tyr::View<tyr::Index<ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition,
                        runir::kr::ps::ext::EvaluationContext<Kind>& context)
{
    const auto value = evaluate(condition.get_feature(), context);

    if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Positive>)
        return value;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Negative>)
        return !value;
    else if constexpr ((std::same_as<FeatureTag, runir::kr::dl::ConceptTag> || std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
                       && std::same_as<ObservationTag, runir::kr::ps::dl::EqualZero>)
        return detail::observation_count<FeatureTag>(value) == 0;
    else if constexpr ((std::same_as<FeatureTag, runir::kr::dl::ConceptTag> || std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
                       && std::same_as<ObservationTag, runir::kr::ps::dl::GreaterZero>)
        return detail::observation_count<FeatureTag>(value) > 0;
}

template<typename FeatureTag, typename ObservationTag, typename C, tyr::planning::TaskKind Kind>
bool is_compatible_with(tyr::View<tyr::Index<ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> effect,
                        runir::kr::ps::dl::EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context)
{
    const auto target = evaluate(effect.get_feature(), context.get_target_context());

    if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Positive>)
        return target;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Negative>)
        return !target;
    else
    {
        const auto source = evaluate(effect.get_feature(), context.get_source_context());

        const auto source_count = detail::observation_count<FeatureTag>(source);
        const auto target_count = detail::observation_count<FeatureTag>(target);

        if constexpr (std::same_as<ObservationTag, runir::kr::ps::dl::Unchanged>)
            return source_count == target_count;
        else if constexpr ((std::same_as<FeatureTag, runir::kr::dl::ConceptTag> || std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
                           && std::same_as<ObservationTag, runir::kr::ps::dl::Increases>)
            return target_count > source_count;
        else if constexpr ((std::same_as<FeatureTag, runir::kr::dl::ConceptTag> || std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
                           && std::same_as<ObservationTag, runir::kr::ps::dl::Decreases>)
            return target_count < source_count;
    }
}

}  // namespace runir::kr::ps::ext

#endif
