#ifndef RUNIR_KR_PS_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_COMPATIBILITY_HPP_

#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/dl/compatibility.hpp"
#include "runir/kr/ps/effect_view.hpp"
#include "runir/kr/ps/family_traits.hpp"
#include "runir/kr/dl/semantics/evaluation_context.hpp"

#include <concepts>
#include <ranges>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps
{

/// A context that conditions and effects of `Family` can be checked against: a state or a transition evaluation context.
template<typename C, typename Family, typename Kind>
concept CompatibilityContext =
    runir::kr::FamilyTag<Family> && tyr::TaskKind<Kind>
    && (runir::kr::dl::semantics::EvaluationContextConcept<C, DlFamilyFor<Family>, Kind> || runir::kr::ps::dl::TransitionEvaluationContextConcept<C, Family, Kind>);

template<tyr::TaskKind Kind,
         runir::kr::FamilyTag Family,
         typename LanguageTag,
         typename C,
         runir::kr::dl::semantics::EvaluationContextConcept<DlFamilyFor<Family>, Kind> Context>
    requires std::same_as<LanguageTag, runir::kr::DlTag>
bool is_compatible_with(ygg::View<ygg::Index<ConcreteConditionVariant<Family, LanguageTag>>, C> condition, Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::is_compatible_with<Kind>(child, context); }, condition.get_variant());
}

template<tyr::TaskKind Kind,
         runir::kr::FamilyTag Family,
         typename C,
         runir::kr::dl::semantics::EvaluationContextConcept<DlFamilyFor<Family>, Kind> Context>
bool is_compatible_with(ygg::View<ygg::Index<ConditionVariant<Family>>, C> condition, Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::is_compatible_with<Kind>(child, context); }, condition.get_variant());
}

template<tyr::TaskKind Kind,
         runir::kr::FamilyTag Family,
         typename LanguageTag,
         typename C,
         runir::kr::ps::dl::TransitionEvaluationContextConcept<Family, Kind> Context>
    requires std::same_as<LanguageTag, runir::kr::DlTag>
bool is_compatible_with(ygg::View<ygg::Index<ConcreteConditionVariant<Family, LanguageTag>>, C> condition, Context& context)
{
    return is_compatible_with<Kind>(condition, context.get_source_context());
}

template<tyr::TaskKind Kind, runir::kr::FamilyTag Family, typename C, runir::kr::ps::dl::TransitionEvaluationContextConcept<Family, Kind> Context>
bool is_compatible_with(ygg::View<ygg::Index<ConditionVariant<Family>>, C> condition, Context& context)
{
    return is_compatible_with<Kind>(condition, context.get_source_context());
}

template<tyr::TaskKind Kind,
         runir::kr::FamilyTag Family,
         typename LanguageTag,
         typename C,
         runir::kr::ps::dl::TransitionEvaluationContextConcept<Family, Kind> Context>
    requires std::same_as<LanguageTag, runir::kr::DlTag>
bool is_compatible_with(ygg::View<ygg::Index<ConcreteEffectVariant<Family, LanguageTag>>, C> effect, Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::is_compatible_with<Kind>(child, context); }, effect.get_variant());
}

template<tyr::TaskKind Kind, runir::kr::FamilyTag Family, typename C, runir::kr::ps::dl::TransitionEvaluationContextConcept<Family, Kind> Context>
bool is_compatible_with(ygg::View<ygg::Index<EffectVariant<Family>>, C> effect, Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::is_compatible_with<Kind>(child, context); }, effect.get_variant());
}

template<tyr::TaskKind Kind, runir::kr::FamilyTag Family, std::ranges::input_range Range, CompatibilityContext<Family, Kind> Context>
bool all_compatible(const Range& values, Context& context)
{
    for (const auto value : values)
        if (!runir::kr::ps::is_compatible_with<Kind>(value, context))
            return false;
    return true;
}

}  // namespace runir::kr::ps

#endif
