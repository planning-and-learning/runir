#ifndef RUNIR_KR_PS_EXT_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_EXT_COMPATIBILITY_HPP_

#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/effect_view.hpp"
#include "runir/kr/ps/ext/dl/compatibility.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"

#include <concepts>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::ext
{

template<typename LanguageTag, typename C, typename Context>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, LanguageTag>>, C> condition,
                        Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context); }, condition.get_variant());
}

template<typename C, typename Context>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>, C> condition, Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context); }, condition.get_variant());
}

template<typename LanguageTag, typename C, typename Context>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, LanguageTag>>, C> effect, Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context); }, effect.get_variant());
}

template<typename C, typename Context>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>, C> effect, Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context); }, effect.get_variant());
}

template<RuleKind Kind, typename C, typename Context>
bool conditions_are_compatible(ygg::View<ygg::Index<Rule<Kind>>, C> rule, Context& context)
{
    for (auto condition : rule.get_conditions())
        if (!runir::kr::ps::ext::is_compatible_with(condition, context))
            return false;

    return true;
}

template<RuleKind Kind, typename C, typename Context>
bool is_compatible_with(ygg::View<ygg::Index<Rule<Kind>>, C> rule, Context& context)
{
    if (!conditions_are_compatible(rule, context))
        return false;

    if constexpr (requires { rule.get_effects(); })
    {
        for (auto effect : rule.get_effects())
            if (!runir::kr::ps::ext::is_compatible_with(effect, context))
                return false;
    }

    return true;
}

template<typename C, typename Context>
bool is_compatible_with(ygg::View<ygg::Index<ps::Rule<ExtFamilyTag>>, C> rule, Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context); }, rule.get_variant());
}

}  // namespace runir::kr::ps::ext

#endif
