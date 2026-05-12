#ifndef RUNIR_KR_GP_DECLARATIONS_HPP_
#define RUNIR_KR_GP_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"

namespace runir::kr::gp
{

template<typename FeatureTypeTag>
struct FeatureTag;

template<typename LangTag, typename FeatureTypeTag>
struct Feature;

struct ConditionVariantTag
{
};

template<typename FeatureTypeTag, typename ObservationTag>
struct ConditionTag;

template<typename LangTag, typename FeatureTypeTag, typename ObservationTag>
struct Condition;

struct EffectVariantTag
{
};

template<typename FeatureTypeTag, typename ChangeTag>
struct EffectTag;

template<typename LangTag, typename FeatureTypeTag, typename ChangeTag>
struct Effect;

struct RuleTag
{
};

struct PolicyTag
{
};

}  // namespace runir::kr::gp

#endif
