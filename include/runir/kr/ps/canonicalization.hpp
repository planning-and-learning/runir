#ifndef RUNIR_KR_PS_CANONICALIZATION_HPP_
#define RUNIR_KR_PS_CANONICALIZATION_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <tyr/common/canonicalization.hpp>

namespace runir::kr::ps
{

template<FamilyTag Family, typename T>
bool is_canonical(const tyr::Data<Feature<Family, T>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename LanguageTag, typename FeatureTag>
bool is_canonical(const tyr::Data<ConcreteFeature<Family, LanguageTag, FeatureTag>>&) noexcept
{
    return true;
}

template<FamilyTag Family>
inline bool is_canonical(const tyr::Data<ConditionVariant<Family>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename LanguageTag>
bool is_canonical(const tyr::Data<ConcreteConditionVariant<Family, LanguageTag>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
bool is_canonical(const tyr::Data<ConcreteCondition<Family, LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{
    return true;
}

template<FamilyTag Family>
inline bool is_canonical(const tyr::Data<EffectVariant<Family>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename LanguageTag>
bool is_canonical(const tyr::Data<ConcreteEffectVariant<Family, LanguageTag>>&) noexcept
{
    return true;
}

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
bool is_canonical(const tyr::Data<ConcreteEffect<Family, LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{
    return true;
}

template<FamilyTag Family>
inline bool is_canonical(const tyr::Data<Rule<Family>>& data) noexcept
{
    return tyr::is_canonical(data.conditions) && tyr::is_canonical(data.effects);
}

template<FamilyTag Family>
inline bool is_canonical(const tyr::Data<Sketch<Family>>& data) noexcept
{
    return tyr::is_canonical(data.rules);
}

template<FamilyTag Family, typename T>
void canonicalize(tyr::Data<Feature<Family, T>>&) noexcept
{
}

template<FamilyTag Family, typename LanguageTag, typename FeatureTag>
void canonicalize(tyr::Data<ConcreteFeature<Family, LanguageTag, FeatureTag>>&) noexcept
{
}

template<FamilyTag Family>
inline void canonicalize(tyr::Data<ConditionVariant<Family>>&) noexcept
{
}

template<FamilyTag Family, typename LanguageTag>
void canonicalize(tyr::Data<ConcreteConditionVariant<Family, LanguageTag>>&) noexcept
{
}

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
void canonicalize(tyr::Data<ConcreteCondition<Family, LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{
}

template<FamilyTag Family>
inline void canonicalize(tyr::Data<EffectVariant<Family>>&) noexcept
{
}

template<FamilyTag Family, typename LanguageTag>
void canonicalize(tyr::Data<ConcreteEffectVariant<Family, LanguageTag>>&) noexcept
{
}

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
void canonicalize(tyr::Data<ConcreteEffect<Family, LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{
}

template<FamilyTag Family>
inline void canonicalize(tyr::Data<Rule<Family>>& data)
{
    tyr::canonicalize(data.conditions);
    tyr::canonicalize(data.effects);
}

template<FamilyTag Family>
inline void canonicalize(tyr::Data<Sketch<Family>>& data)
{
    tyr::canonicalize(data.rules);
}

}  // namespace runir::kr::ps

#endif
