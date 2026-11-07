#ifndef RUNIR_KR_PS_DECLARATIONS_HPP_
#define RUNIR_KR_PS_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/register_values_view_concepts.hpp"

#include <concepts>
#include <memory>
#include <tyr/planning/state_view_concepts.hpp>
#include <urd/elements/flat.hpp>

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

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct ConcreteCondition;

// Effect

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct ConcreteEffect;

// Repository

template<FamilyTag Family, typename RepositoryTypes>
class BasicRepository;

template<FamilyTag Family, typename RepositoryTypes>
class BasicRepositoryFactory;

/// A family's own types, specialized in the family's declarations.hpp: `Repository` for the policy families; the
/// program families (ext, icp) also name `ConcreteRuleTypes`, `ExecutionRepositoryTypes<Kind>` and `ProgramState<Kind>`.
template<FamilyTag Family>
struct FamilyTypes;

template<FamilyTag Family>
using Repository = typename FamilyTypes<Family>::Repository;

/// A rule of the family's sketch (base) or a rule variant of the family's modules (ext, icp).
template<FamilyTag Family>
using RuleView = urd::index_view<Rule<Family>, Repository<Family>>;

template<FamilyTag Family, typename FeatureTag>
using FeatureView = urd::index_view<Feature<Family, FeatureTag>, Repository<Family>>;

// Programs: modules over memory states, executed on program states.

template<typename T>
concept ProgramFamilyTag = std::same_as<T, ExtFamilyTag> || std::same_as<T, IcpFamilyTag>;

template<ProgramFamilyTag Family>
struct MemoryState;

template<ProgramFamilyTag Family>
struct ModuleSymbol;

template<ProgramFamilyTag Family, tyr::TaskKind Kind>
class ExecutionRepository;

template<ProgramFamilyTag Family, tyr::TaskKind Kind>
class ExecutionRepositoryFactory;

template<ProgramFamilyTag Family, tyr::TaskKind Kind>
using ExecutionRepositoryPtr = std::shared_ptr<ExecutionRepository<Family, Kind>>;

template<ProgramFamilyTag Family, tyr::TaskKind Kind>
using ProgramStateView = urd::index_view<typename FamilyTypes<Family>::template ProgramState<Kind>, ExecutionRepository<Family, Kind>>;

}  // namespace runir::kr::ps

#endif
