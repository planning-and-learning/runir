#ifndef RUNIR_KR_PS_ICP_CANONICALIZATION_HPP_
#define RUNIR_KR_PS_ICP_CANONICALIZATION_HPP_

#include "runir/kr/ps/icp/datas.hpp"

#include <algorithm>
#include "runir/kr/ps/canonicalization.hpp"

#include <yggdrasil/semantics/canonicalization.hpp>

namespace runir::kr::ps
{

inline bool is_canonical(const urd::data<Rule<IcpFamilyTag>>&) noexcept { return true; }

inline void canonicalize(urd::data<Rule<IcpFamilyTag>>&) noexcept {}

}

namespace runir::kr::ps::icp
{

template<typename T>
bool is_canonical(const urd::data<T>&) noexcept
{ return true; }

template<typename T>
void canonicalize(urd::data<T>&) noexcept
{
}

template<RuleKind Kind>
bool is_canonical(const urd::data<Rule<Kind>>& data) noexcept
{
    bool result = ygg::is_canonical(data.conditions) && ygg::is_canonical(data.effects);
    if constexpr (std::same_as<Kind, CruleTag>)
        result = result && ygg::is_canonical(data.xconditions) && ygg::is_canonical(data.xeffects);
    return result;
}

template<RuleKind Kind>
void canonicalize(urd::data<Rule<Kind>>& data)
{
    ygg::canonicalize(data.conditions);
    ygg::canonicalize(data.effects);
    if constexpr (std::same_as<Kind, CruleTag>)
    {
        ygg::canonicalize(data.xconditions);
        ygg::canonicalize(data.xeffects);
    }
}

inline bool is_canonical(const urd::data<Module>& data) noexcept
{
    return ygg::is_canonical(data.concept_features) && ygg::is_canonical(data.role_features) && ygg::is_canonical(data.boolean_features)
           && ygg::is_canonical(data.numerical_features) && ygg::is_canonical(data.query_features) && ygg::is_canonical(data.memory_states)
           && std::all_of(data.memory_transitions.begin(), data.memory_transitions.end(), [](const auto& row) { return ygg::is_canonical(row); })
           && std::is_sorted(data.reset_pairs.begin(), data.reset_pairs.end())
           && std::adjacent_find(data.reset_pairs.begin(), data.reset_pairs.end()) == data.reset_pairs.end();
}

inline void canonicalize(urd::data<Module>& data)
{
    ygg::canonicalize(data.concept_features);
    ygg::canonicalize(data.role_features);
    ygg::canonicalize(data.boolean_features);
    ygg::canonicalize(data.numerical_features);
    ygg::canonicalize(data.query_features);
    ygg::canonicalize(data.memory_states);
    for (auto& row : data.memory_transitions)
        ygg::canonicalize(row);
    std::sort(data.reset_pairs.begin(), data.reset_pairs.end());
    data.reset_pairs.erase(std::unique(data.reset_pairs.begin(), data.reset_pairs.end()), data.reset_pairs.end());
}

}

#endif
