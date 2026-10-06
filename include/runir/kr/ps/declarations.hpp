#ifndef RUNIR_KR_PS_DECLARATIONS_HPP_
#define RUNIR_KR_PS_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"

#include <tyr/planning/state_view.hpp>

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
template<FamilyTag Family,
         tyr::TaskKind Kind,
         tyr::planning::StateViewConcept<Kind> S = tyr::planning::StateView<Kind>,
         runir::kr::dl::semantics::RegisterValuesViewConcept R = runir::kr::dl::semantics::RegisterValuesView>
class TransitionEvaluationContext;
}

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
