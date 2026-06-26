#ifndef RUNIR_KR_PS_EXT_DECLARATIONS_HPP_
#define RUNIR_KR_PS_EXT_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/declarations.hpp"

#include <concepts>

namespace runir::kr::ps::ext
{

template<runir::kr::dl::CategoryTag Category>
    requires(std::same_as<Category, runir::kr::dl::ConceptTag> || std::same_as<Category, runir::kr::dl::RoleTag>)
struct Register
{
};

struct MemoryState
{
};

template<runir::kr::dl::CategoryTag Category>
struct Argument
{
};

// Features

template<typename FeatureTag>
struct Feature
{
};

template<typename LanguageTag, typename FeatureTag>
struct ConcreteFeature
{
};

// Conditions

struct ConditionVariant
{
};

template<typename LanguageTag>
struct ConcreteConditionVariant
{
};

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct ConcreteCondition
{
};

// Effects

struct EffectVariant
{
};

template<typename LanguageTag>
struct ConcreteEffectVariant
{
};

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct ConcreteEffect
{
};

// Rules

struct SketchTag
{
    static constexpr auto keyword = "sketch";
};

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
concept RuleKind = std::same_as<T, LoadTag> || std::same_as<T, SketchTag> || std::same_as<T, DoTag> || std::same_as<T, CallTag>;

template<RuleKind Kind, typename Category = void>
struct Rule
{
};

struct RuleVariant
{
};

struct Module
{
};

struct ModuleProgram
{
};

}  // namespace runir::kr::ps::ext

#endif
