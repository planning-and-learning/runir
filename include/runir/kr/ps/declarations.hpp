#ifndef RUNIR_KR_PS_DECLARATIONS_HPP_
#define RUNIR_KR_PS_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"

#include <type_traits>

namespace runir::kr::ps
{

using runir::kr::FamilyTag;

// Rule

template<FamilyTag Family>
struct Rule;

// Feature

template<FamilyTag Family, typename FeatureTag>
struct Feature;

template<FamilyTag Family, typename LanguageTag, typename FeatureTag>
struct ConcreteFeature;

// Transition evaluation

namespace dl
{
template<FamilyTag Family, tyr::TaskKind Kind>
class TransitionEvaluationContext;
}

template<typename Family, typename LanguageTag, typename Context>
concept IsTransitionEvaluationContext =
    FamilyTag<Family> && std::same_as<LanguageTag, runir::kr::DlTag>
    && (std::same_as<std::remove_cvref_t<Context>, dl::TransitionEvaluationContext<Family, tyr::GroundTag>>
        || std::same_as<std::remove_cvref_t<Context>, dl::TransitionEvaluationContext<Family, tyr::LiftedTag>>);

// Condition

template<FamilyTag Family>
struct ConditionVariant
{
};

template<FamilyTag Family, typename LanguageTag>
struct ConcreteConditionVariant;

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct ConcreteCondition;

// Effect

template<FamilyTag Family>
struct EffectVariant
{
};

template<FamilyTag Family, typename LanguageTag>
struct ConcreteEffectVariant;

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct ConcreteEffect;

// Repository

template<FamilyTag Family, typename RepositoryTypes>
class BasicRepository;

template<FamilyTag Family, typename RepositoryTypes>
class BasicRepositoryFactory;

}  // namespace runir::kr::ps

#endif
