#ifndef RUNIR_KR_PS_CANONICALIZATION_HPP_
#define RUNIR_KR_PS_CANONICALIZATION_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <algorithm>
#include <yggdrasil/semantics/canonicalization.hpp>

namespace runir::kr::ps
{

template<FamilyTag Family, typename T>
bool is_canonical(const urd::data<Feature<Family, T>>&) noexcept
{ return true; }

template<FamilyTag Family, typename LanguageTag, typename FeatureTag>
bool is_canonical(const urd::data<ConcreteFeature<Family, LanguageTag, FeatureTag>>&) noexcept
{ return true; }

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
bool is_canonical(const urd::data<ConcreteCondition<Family, LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{ return true; }

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
bool is_canonical(const urd::data<ConcreteEffect<Family, LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{ return true; }

template<FamilyTag Family, typename T>
void canonicalize(urd::data<Feature<Family, T>>&) noexcept
{
}

template<FamilyTag Family, typename LanguageTag, typename FeatureTag>
void canonicalize(urd::data<ConcreteFeature<Family, LanguageTag, FeatureTag>>&) noexcept
{
}

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
void canonicalize(urd::data<ConcreteCondition<Family, LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{
}

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
void canonicalize(urd::data<ConcreteEffect<Family, LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{
}

}  // namespace runir::kr::ps

#endif
