#ifndef RUNIR_KR_PS_EXT_DECLARATIONS_HPP_
#define RUNIR_KR_PS_EXT_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"

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

// Features/conditions/effects use the generic policy-sketch wrappers. The ext namespace keeps
// aliases for the module-program API surface while storage/evaluation stay family-parametric.

template<typename FeatureTag>
using Feature = runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>;

template<typename LanguageTag, typename FeatureTag>
using ConcreteFeature = runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, LanguageTag, FeatureTag>;

using ConditionVariant = runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>;

template<typename LanguageTag>
using ConcreteConditionVariant = runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, LanguageTag>;

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
using ConcreteCondition = runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, LanguageTag, FeatureTag, ObservationTag>;

using EffectVariant = runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>;

template<typename LanguageTag>
using ConcreteEffectVariant = runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, LanguageTag>;

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
using ConcreteEffect = runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, LanguageTag, FeatureTag, ObservationTag>;

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
