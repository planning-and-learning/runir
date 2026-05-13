#ifndef RUNIR_KR_GP_CANONICALIZATION_HPP_
#define RUNIR_KR_GP_CANONICALIZATION_HPP_

#include "runir/kr/gp/condition_data.hpp"
#include "runir/kr/gp/dl/condition_data.hpp"
#include "runir/kr/gp/dl/effect_data.hpp"
#include "runir/kr/gp/dl/feature_data.hpp"
#include "runir/kr/gp/effect_data.hpp"
#include "runir/kr/gp/feature_data.hpp"
#include "runir/kr/gp/policy_data.hpp"
#include "runir/kr/gp/rule_data.hpp"

#include <tyr/common/canonicalization.hpp>

namespace runir::kr::gp
{

template<typename T>
bool is_canonical(const tyr::Data<Feature<T>>&) noexcept
{
    return true;
}

template<typename LanguageTag, typename FeatureTag>
bool is_canonical(const tyr::Data<ConcreteFeature<LanguageTag, FeatureTag>>&) noexcept
{
    return true;
}

inline bool is_canonical(const tyr::Data<ConditionVariant>&) noexcept { return true; }

template<typename LanguageTag>
bool is_canonical(const tyr::Data<ConcreteConditionVariant<LanguageTag>>&) noexcept
{
    return true;
}

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
bool is_canonical(const tyr::Data<ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{
    return true;
}

inline bool is_canonical(const tyr::Data<EffectVariant>&) noexcept { return true; }

template<typename LanguageTag>
bool is_canonical(const tyr::Data<ConcreteEffectVariant<LanguageTag>>&) noexcept
{
    return true;
}

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
bool is_canonical(const tyr::Data<ConcreteEffect<LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{
    return true;
}

inline bool is_canonical(const tyr::Data<Rule>& data) noexcept { return tyr::is_canonical(data.conditions) && tyr::is_canonical(data.effects); }

inline bool is_canonical(const tyr::Data<Policy>& data) noexcept { return tyr::is_canonical(data.rules); }

template<typename T>
void canonicalize(tyr::Data<Feature<T>>&) noexcept
{
}

template<typename LanguageTag, typename FeatureTag>
void canonicalize(tyr::Data<ConcreteFeature<LanguageTag, FeatureTag>>&) noexcept
{
}

inline void canonicalize(tyr::Data<ConditionVariant>&) noexcept {}

template<typename LanguageTag>
void canonicalize(tyr::Data<ConcreteConditionVariant<LanguageTag>>&) noexcept
{
}

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
void canonicalize(tyr::Data<ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{
}

inline void canonicalize(tyr::Data<EffectVariant>&) noexcept {}

template<typename LanguageTag>
void canonicalize(tyr::Data<ConcreteEffectVariant<LanguageTag>>&) noexcept
{
}

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
void canonicalize(tyr::Data<ConcreteEffect<LanguageTag, FeatureTag, ObservationTag>>&) noexcept
{
}

inline void canonicalize(tyr::Data<Rule>& data)
{
    tyr::canonicalize(data.conditions);
    tyr::canonicalize(data.effects);
}

inline void canonicalize(tyr::Data<Policy>& data) { tyr::canonicalize(data.rules); }

}  // namespace runir::kr::gp

#endif
