#ifndef RUNIR_KR_PS_DECLARATIONS_HPP_
#define RUNIR_KR_PS_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/register_values_view_concepts.hpp"

#include <concepts>
#include <memory>
#include <tyr/planning/state_view_concepts.hpp>
#include <urd/elements/concepts.hpp>
#include <urd/elements/flat.hpp>

namespace runir::kr::ps
{

using runir::kr::FamilyTag;

/// Whether a rule (its view) leads to a target memory state. Every rule does but a family's guards, which the family
/// states (ext: Backtrack, ext/declarations.hpp).
template<typename RuleView>
inline constexpr bool transition_rule = true;

// Rule

template<FamilyTag Family>
struct Rule
{
    static constexpr std::string_view name = urd::joined_name<"Rule", Family::name>;
};

// Feature

template<FamilyTag Family, typename FeatureTag>
struct Feature
{
    static constexpr std::string_view name = urd::joined_name<"Feature", Family::name, FeatureTag::name>;
};

template<FamilyTag Family, typename FeatureTag>
struct ConcreteFeature
{
    static constexpr std::string_view name = urd::joined_name<"ConcreteFeature", Family::name, FeatureTag::name>;
};

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

template<FamilyTag Family, typename FeatureTag, typename ObservationTag>
struct ConcreteCondition
{
    static constexpr std::string_view name = urd::joined_name<"ConcreteCondition", Family::name, FeatureTag::name, ObservationTag::name>;
};

// Effect

template<FamilyTag Family, typename FeatureTag, typename ObservationTag>
struct ConcreteEffect
{
    static constexpr std::string_view name = urd::joined_name<"ConcreteEffect", Family::name, FeatureTag::name, ObservationTag::name>;
};

// Repository

template<FamilyTag Family, typename RepositoryTypes>
class BasicRepository;

/// A family's own types, specialized in the family's declarations.hpp: `Repository` for the policy families; the
/// program families (ext, icp) also name `ConcreteRuleTypes`, `ControlRepositoryTypes`, `ExecutionRepositoryTypes<Kind>` and
/// `ProgramState<Kind>`.
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
struct MemoryState
{
    static constexpr std::string_view name = urd::joined_name<"MemoryState", Family::name>;
};

template<ProgramFamilyTag Family>
struct ModuleSymbol
{
    static constexpr std::string_view name = urd::joined_name<"ModuleSymbol", Family::name>;
};

/// A task's records of a program's control that hold no planning state (activations, frames, histories), whatever the
/// task's kind; its execution repository refers into it.
template<ProgramFamilyTag Family>
class ControlRepository;

template<ProgramFamilyTag Family>
using ControlRepositoryPtr = std::shared_ptr<ControlRepository<Family>>;

template<ProgramFamilyTag Family, tyr::TaskKind Kind>
class ExecutionRepository;

template<ProgramFamilyTag Family, tyr::TaskKind Kind>
using ExecutionRepositoryPtr = std::shared_ptr<ExecutionRepository<Family, Kind>>;

template<ProgramFamilyTag Family, tyr::TaskKind Kind>
using ProgramStateView = urd::index_view<typename FamilyTypes<Family>::template ProgramState<Kind>, ExecutionRepository<Family, Kind>>;

}  // namespace runir::kr::ps

#endif
