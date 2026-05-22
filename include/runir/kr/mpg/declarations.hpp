#ifndef RUNIR_KR_MPG_DECLARATIONS_HPP_
#define RUNIR_KR_MPG_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"

#include <concepts>

namespace runir::kr::mpg
{

struct Register
{
};

struct DFA
{
};

struct AutomataState
{
};

// Feature

template<typename FeatureTag>
struct Feature;

template<typename LanguageTag, typename FeatureTag>
struct ConcreteFeature;

// EvaluationContext

template<typename LanguageTag>
struct EvaluationContext;

template<typename LanguageTag, typename Context>
concept IsEvaluationContext = requires(const Context& context) {
    context.get_source_state();
    context.get_target_state();
};

// Condition

struct ConditionVariant
{
};

template<typename LanguageTag>
struct ConcreteConditionVariant;

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct ConcreteCondition;

// Effect

struct EffectVariant
{
};

template<typename LanguageTag>
struct ConcreteEffectVariant;

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct ConcreteEffect;

// Rules

struct LoadTag
{
    static constexpr auto keyword = "load";
};

struct DoTag
{
    static constexpr auto keyword = "do";
};

struct CallTag
{
    static constexpr auto keyword = "call";
};

template<typename T>
concept RuleKind = std::same_as<T, LoadTag> || std::same_as<T, DoTag> || std::same_as<T, CallTag>;

template<RuleKind Kind>
struct Rule
{
};

struct Policy
{
};

struct Module
{
};

}  // namespace runir::kr::mpg

#endif
