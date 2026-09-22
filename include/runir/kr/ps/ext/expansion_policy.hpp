#ifndef RUNIR_KR_PS_EXT_EXPANSION_POLICY_HPP_
#define RUNIR_KR_PS_EXT_EXPANSION_POLICY_HPP_

#include <concepts>

namespace runir::kr::ps::ext
{

struct EagerExpansionPolicy
{
};

struct LazyExpansionPolicy
{
};

template<typename T>
concept ExpansionPolicy = std::same_as<T, EagerExpansionPolicy> || std::same_as<T, LazyExpansionPolicy>;

}  // namespace runir::kr::ps::ext

#endif
