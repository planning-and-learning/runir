#ifndef RUNIR_KR_PS_EXT_CANONICALIZATION_HPP_
#define RUNIR_KR_PS_EXT_CANONICALIZATION_HPP_

#include "runir/kr/ps/ext/module_data.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/kr/ps/ext/rule_variant_data.hpp"

#include <algorithm>
#include <tyr/common/canonicalization.hpp>
#include <tyr/common/comparators.hpp>
#include <tyr/common/equal_to.hpp>

namespace runir::kr::ps::ext
{

template<RuleKind Kind>
bool is_canonical(const tyr::Data<Rule<Kind>>& data) noexcept
{
    return tyr::is_canonical(data.conditions) && tyr::is_canonical(data.effects);
}

inline bool is_canonical(const tyr::Data<RuleVariant>&) noexcept { return true; }

inline bool is_canonical(const MemoryTransition& transition) noexcept { return tyr::is_canonical(transition.rules); }

template<typename Transitions>
bool is_canonical_memory_transitions(const Transitions& transitions) noexcept
{
    return std::is_sorted(transitions.begin(), transitions.end(), tyr::Less<MemoryTransition> {})
           && std::all_of(transitions.begin(), transitions.end(), [](const auto& transition) { return is_canonical(transition); });
}

inline bool is_canonical(const tyr::Data<Module>& data) noexcept
{
    return tyr::is_canonical(data.registers) && tyr::is_canonical(data.memory_states) && is_canonical_memory_transitions(data.memory_transitions);
}

template<RuleKind Kind>
void canonicalize(tyr::Data<Rule<Kind>>& data)
{
    tyr::canonicalize(data.conditions);
    tyr::canonicalize(data.effects);
}

inline void canonicalize(tyr::Data<RuleVariant>&) noexcept {}

inline void canonicalize(MemoryTransition& transition) { tyr::canonicalize(transition.rules); }

template<typename Transitions>
void canonicalize_memory_transitions(Transitions& transitions)
{
    for (auto& transition : transitions)
        canonicalize(transition);

    if (!std::is_sorted(transitions.begin(), transitions.end(), tyr::Less<MemoryTransition> {}))
        std::sort(transitions.begin(), transitions.end(), tyr::Less<MemoryTransition> {});

    transitions.erase(std::unique(transitions.begin(), transitions.end(), tyr::EqualTo<MemoryTransition> {}), transitions.end());
}

inline void canonicalize(tyr::Data<Module>& data)
{
    tyr::canonicalize(data.registers);
    tyr::canonicalize(data.memory_states);
    canonicalize_memory_transitions(data.memory_transitions);
}

}  // namespace runir::kr::ps::ext

#endif
