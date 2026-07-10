#ifndef RUNIR_KR_PS_EXT_DL_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_EXT_DL_COMPATIBILITY_HPP_

#include "runir/kr/ps/ext/dl/condition_view.hpp"
#include "runir/kr/ps/ext/dl/effect_view.hpp"
#include "runir/kr/ps/ext/dl/evaluation.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"

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

template<typename FeatureTag, typename ObservationTag, typename C, tyr::planning::TaskKind Kind>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition,
                        runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context)
{
    const auto value = evaluate(condition.get_feature(), context);
    return detail::condition_matches<FeatureTag, ObservationTag>(value);
}

template<typename FeatureTag, typename ObservationTag, typename C, tyr::planning::TaskKind Kind>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition,
                        runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context,
                        runir::kr::dl::semantics::EvaluationWorkspace& workspace)
{
    const auto value = evaluate(condition.get_feature(), context, workspace);
    return detail::condition_matches<FeatureTag, ObservationTag>(value);
}

template<typename FeatureTag, typename ObservationTag, typename C, tyr::planning::TaskKind Kind>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition,
                        runir::kr::ps::dl::EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context)
{
    return is_compatible_with(condition, context.get_source_context(), context.get_workspace());
}

template<typename FeatureTag, typename ObservationTag, typename C, tyr::planning::TaskKind Kind>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition,
                        runir::kr::ps::ext::EvaluationContext<Kind>& context,
                        runir::kr::ps::ext::EvaluationEnvironment<Kind>& environment)
{
    auto dl_context = environment.make_dl_context(context);
    return is_compatible_with(condition, dl_context, environment.get_dl_workspace());
}

template<typename FeatureTag, typename ObservationTag, typename C, tyr::planning::TaskKind Kind>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> effect,
                        runir::kr::ps::dl::EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context)
{
    auto& workspace = context.get_workspace();
    const auto target = evaluate(effect.get_feature(), context.get_target_context(), workspace);

    if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Positive>)
        return target;
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature> && std::same_as<ObservationTag, runir::kr::ps::dl::Negative>)
        return !target;
    else
    {
        const auto source = evaluate(effect.get_feature(), context.get_source_context(), workspace);

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
