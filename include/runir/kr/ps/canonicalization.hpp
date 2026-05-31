#ifndef RUNIR_KR_PS_CANONICALIZATION_HPP_
#define RUNIR_KR_PS_CANONICALIZATION_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <yggdrasil/semantics/canonicalization.hpp>

namespace runir::kr::ps
{

template<FamilyTag Family, typename T>
bool is_canonical(const ygg::Data<Feature<Family, T>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename LanguageTag, typename FeatureTag>
bool is_canonical(const ygg::Data<ConcreteFeature<Family, LanguageTag, FeatureTag>>&) noexcept
{
    return true;
}

template<FamilyTag Family>
inline bool is_canonical(const ygg::Data<ConditionVariant<Family>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename LanguageTag>
bool is_canonical(const ygg::Data<ConcreteConditionVariant<Family, LanguageTag>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
bool is_canonical(const ygg::Data<ConcreteCondition<Family, LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{
    return true;
}

template<FamilyTag Family>
inline bool is_canonical(const ygg::Data<EffectVariant<Family>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename LanguageTag>
bool is_canonical(const ygg::Data<ConcreteEffectVariant<Family, LanguageTag>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
bool is_canonical(const ygg::Data<ConcreteEffect<Family, LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename T>
void canonicalize(ygg::Data<Feature<Family, T>>&) noexcept
{
}

template<FamilyTag Family, typename LanguageTag, typename FeatureTag>
void canonicalize(ygg::Data<ConcreteFeature<Family, LanguageTag, FeatureTag>>&) noexcept
{
}

template<FamilyTag Family>
inline void canonicalize(ygg::Data<ConditionVariant<Family>>&) noexcept
{
}

template<FamilyTag Family, typename LanguageTag>
void canonicalize(ygg::Data<ConcreteConditionVariant<Family, LanguageTag>>&) noexcept
{
}

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
void canonicalize(ygg::Data<ConcreteCondition<Family, LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{
}

template<FamilyTag Family>
inline void canonicalize(ygg::Data<EffectVariant<Family>>&) noexcept
{
}

template<FamilyTag Family, typename LanguageTag>
void canonicalize(ygg::Data<ConcreteEffectVariant<Family, LanguageTag>>&) noexcept
{
}

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
void canonicalize(ygg::Data<ConcreteEffect<Family, LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{
}

}  // namespace runir::kr::ps

#endif
