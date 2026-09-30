#ifndef RUNIR_KR_PS_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_COMPATIBILITY_HPP_

#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/dl/compatibility.hpp"
#include "runir/kr/ps/effect_view.hpp"

#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps
{

template<runir::kr::FamilyTag Family,
         typename LanguageTag,
         typename C,
         runir::kr::dl::semantics::StateEvaluationContextConcept<typename PsFamilyTraits<Family>::DlFamily> Context>
    requires std::same_as<LanguageTag, runir::kr::DlTag>
bool is_compatible_with(ygg::View<ygg::Index<ConcreteConditionVariant<Family, LanguageTag>>, C> condition, Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::is_compatible_with(child, context); }, condition.get_variant());
}

template<runir::kr::FamilyTag Family, typename C, runir::kr::dl::semantics::StateEvaluationContextConcept<typename PsFamilyTraits<Family>::DlFamily> Context>
bool is_compatible_with(ygg::View<ygg::Index<ConditionVariant<Family>>, C> condition, Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::is_compatible_with(child, context); }, condition.get_variant());
}

template<runir::kr::FamilyTag Family, typename LanguageTag, typename C, runir::kr::ps::dl::TransitionEvaluationContextConcept<Family> Context>
    requires std::same_as<LanguageTag, runir::kr::DlTag>
bool is_compatible_with(ygg::View<ygg::Index<ConcreteConditionVariant<Family, LanguageTag>>, C> condition, Context& context)
{
    return is_compatible_with(condition, context.get_source_context());
}

template<runir::kr::FamilyTag Family, typename C, runir::kr::ps::dl::TransitionEvaluationContextConcept<Family> Context>
bool is_compatible_with(ygg::View<ygg::Index<ConditionVariant<Family>>, C> condition, Context& context)
{
    return is_compatible_with(condition, context.get_source_context());
}

template<runir::kr::FamilyTag Family, typename LanguageTag, typename C, runir::kr::ps::dl::TransitionEvaluationContextConcept<Family> Context>
    requires std::same_as<LanguageTag, runir::kr::DlTag>
bool is_compatible_with(ygg::View<ygg::Index<ConcreteEffectVariant<Family, LanguageTag>>, C> effect, Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::is_compatible_with(child, context); }, effect.get_variant());
}

template<runir::kr::FamilyTag Family, typename C, runir::kr::ps::dl::TransitionEvaluationContextConcept<Family> Context>
bool is_compatible_with(ygg::View<ygg::Index<EffectVariant<Family>>, C> effect, Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::is_compatible_with(child, context); }, effect.get_variant());
}

}  // namespace runir::kr::ps

#endif
