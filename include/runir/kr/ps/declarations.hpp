#ifndef RUNIR_KR_PS_DECLARATIONS_HPP_
#define RUNIR_KR_PS_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/register_values_view_concepts.hpp"

#include <concepts>
#include <memory>
#include <string_view>
#include <tyr/planning/states/state_view_concepts.hpp>
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
         tyr::planning::StateBuilderViewConcept<Kind> S = tyr::planning::SharedStateBuilderView<Kind>,
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

template<typename Derived, FamilyTag Family, typename RepositoryTypes>
class BasicRepository;

/// Whether R is a family's repository (a class deriving from BasicRepository): its Family and its RepositoryTypes.
template<typename R>
concept FamilyRepository = FamilyTag<typename R::Family> && requires { typename R::RepositoryTypes; };

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

/// The labels of its proof graphs' vertices and edges (records: data<...>, the graph's elements).
template<runir::kr::ps::ProgramFamilyTag Family, tyr::TaskKind Kind>
struct ProgramProofVertexLabel
{
    static constexpr std::string_view name = urd::joined_name<"ProgramProofVertexLabel", Family::name, Kind::name>;
};

template<runir::kr::ps::ProgramFamilyTag Family>
struct ProgramProofEdgeLabel
{
    static constexpr std::string_view name = urd::joined_name<"ProgramProofEdgeLabel", Family::name>;
};
}  // namespace runir::kr::ps

#endif
