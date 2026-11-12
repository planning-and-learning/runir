#ifndef RUNIR_KR_PS_EXT_CANONICALIZATION_HPP_
#define RUNIR_KR_PS_EXT_CANONICALIZATION_HPP_

#include "runir/kr/ps/canonicalization.hpp"
#include "runir/kr/ps/dl/condition_data.hpp"
#include "runir/kr/ps/dl/effect_data.hpp"
#include "runir/kr/ps/ext/module_data.hpp"
#include "runir/kr/ps/ext/program_data.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/kr/ps/feature_data.hpp"
#include "runir/kr/ps/memory_state_data.hpp"
#include "runir/kr/ps/module_symbol_data.hpp"
#include "runir/kr/ps/rule_variant_data.hpp"

#include <algorithm>
#include <yggdrasil/semantics/canonicalization.hpp>

namespace runir::kr::ps::ext
{

inline bool is_canonical(const urd::data<OrderTerm>&) noexcept { return true; }

template<RuleKind Kind>
bool is_canonical(const urd::data<Rule<Kind>>& data) noexcept
{
    if constexpr (BindingRuleKind<Kind> || std::same_as<Kind, SketchTag> || std::same_as<Kind, DoTag> || std::same_as<Kind, ActionTag>)
        return ygg::is_canonical(data.conditions) && ygg::is_canonical(data.effects);
    else if constexpr (std::same_as<Kind, CallTag> || std::same_as<Kind, BacktrackTag>)
        return ygg::is_canonical(data.conditions);
}

inline bool is_canonical(const urd::data<Module>& data) noexcept
{
    return ygg::is_canonical(data.concept_features) && ygg::is_canonical(data.role_features) && ygg::is_canonical(data.boolean_features)
           && ygg::is_canonical(data.numerical_features) && ygg::is_canonical(data.query_features) && ygg::is_canonical(data.memory_states);
}

inline bool is_canonical(const urd::data<Program>&) noexcept { return true; }

inline void canonicalize(urd::data<OrderTerm>&) noexcept {}

template<RuleKind Kind>
void canonicalize(urd::data<Rule<Kind>>& data)
{
    if constexpr (BindingRuleKind<Kind> || std::same_as<Kind, SketchTag> || std::same_as<Kind, DoTag> || std::same_as<Kind, ActionTag>)
    {
        ygg::canonicalize(data.conditions);
        ygg::canonicalize(data.effects);
    }
    else if constexpr (std::same_as<Kind, CallTag> || std::same_as<Kind, BacktrackTag>)
    {
        ygg::canonicalize(data.conditions);
    }
}

inline void canonicalize(urd::data<Module>& data)
{
    ygg::canonicalize(data.concept_features);
    ygg::canonicalize(data.role_features);
    ygg::canonicalize(data.boolean_features);
    ygg::canonicalize(data.numerical_features);
    ygg::canonicalize(data.query_features);
    ygg::canonicalize(data.memory_states);
}

inline void canonicalize(urd::data<Program>&) noexcept {}

}  // namespace runir::kr::ps::ext

#endif
