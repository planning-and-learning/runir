#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DELTA_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DELTA_HPP_

#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <tyr/planning/state_view.hpp>
#include <utility>
#include <vector>

namespace runir::kr::dl::semantics::incremental
{

namespace detail
{
template<tyr::TaskKind Kind, tyr::planning::StateViewConcept<Kind> Source, tyr::planning::StateViewConcept<Kind> Target, typename Changes>
void assign_atom_delta(const Source& source, const Target& target, Changes& delta)
{
    if (&source.get_task() != &target.get_task())
        throw std::invalid_argument("Feature delta requires states from the same task.");
    delta.clear();
    // ponytail: compare dynamic state contents for now; capture changes during
    // successor construction if this scan becomes a measured bottleneck.
    for (const auto atom : source.template get_atoms_view<tyr::formalism::FluentTag>())
        if (!target.test(atom))
            delta.removed.fluent_atoms.push_back(atom);
    for (const auto atom : target.template get_atoms_view<tyr::formalism::FluentTag>())
        if (!source.test(atom))
            delta.added.fluent_atoms.push_back(atom);
    for (const auto atom : source.template get_atoms_view<tyr::formalism::DerivedTag>())
        if (!target.test(atom))
            delta.removed.derived_atoms.push_back(atom);
    for (const auto atom : target.template get_atoms_view<tyr::formalism::DerivedTag>())
        if (!source.test(atom))
            delta.added.derived_atoms.push_back(atom);
}
}  // namespace detail

/// Changes to fluent and derived atoms. Static facts, goals and the object universe
/// are task constants; planning numeric values are not current DL inputs.
/// The task repository must outlive the delta; state buffers need not.
template<FamilyTag Family>
struct Delta
{
    struct Values
    {
        std::vector<tyr::formalism::planning::AtomView<tyr::GroundTag, tyr::formalism::FluentTag>> fluent_atoms;
        std::vector<tyr::formalism::planning::AtomView<tyr::GroundTag, tyr::formalism::DerivedTag>> derived_atoms;

        void clear() noexcept
        {
            fluent_atoms.clear();
            derived_atoms.clear();
        }
    };

    Values added;
    Values removed;

    template<tyr::TaskKind Kind, tyr::planning::StateViewConcept<Kind> Source, tyr::planning::StateViewConcept<Kind> Target>
    void assign(const Source& source, const Target& target)
    {
        detail::assign_atom_delta<Kind>(source, target, *this);
    }

    void clear() noexcept
    {
        added.clear();
        removed.clear();
    }
    bool empty() const noexcept
    {
        return added.fluent_atoms.empty() && removed.fluent_atoms.empty() && added.derived_atoms.empty() && removed.derived_atoms.empty();
    }
    void reverse() noexcept { std::swap(added, removed); }
};

/// Changes to Ext feature inputs within one module invocation.
/// Calls, returns and backtracking across invocations require reinitializing
/// dynamic feature results from the destination context; task-static results stay reusable.
/// Arguments are fixed within an invocation and have no delta representation.
/// Static facts, the goal and object universe are also excluded as task constants.
/// Planning numeric values are not inputs of the current DL constructors;
/// this delta does not describe an entire planning or program state.
/// The task formalism repository must outlive the delta. State and register
/// buffers need not: each stored view identifies an immutable repository object.
template<>
struct Delta<runir::kr::ExtFamilyTag>
{
    struct Values
    {
        std::vector<tyr::formalism::planning::AtomView<tyr::GroundTag, tyr::formalism::FluentTag>> fluent_atoms;
        std::vector<tyr::formalism::planning::AtomView<tyr::GroundTag, tyr::formalism::DerivedTag>> derived_atoms;
        std::vector<std::pair<RegisterIdentifier<ConceptTag>, tyr::formalism::planning::ObjectView>> concept_registers;
        std::vector<std::pair<RegisterIdentifier<RoleTag>, std::pair<tyr::formalism::planning::ObjectView, tyr::formalism::planning::ObjectView>>>
            role_registers;

        void clear() noexcept
        {
            fluent_atoms.clear();
            derived_atoms.clear();
            concept_registers.clear();
            role_registers.clear();
        }
    };

    Values added;
    Values removed;

    /// Refill retained buffers from completed states, including their derived atoms.
    /// Both endpoints must belong to the same module invocation.
    template<tyr::TaskKind Kind,
             tyr::planning::StateViewConcept<Kind> Source,
             RegisterValuesViewConcept SourceRegisters,
             tyr::planning::StateViewConcept<Kind> Target,
             RegisterValuesViewConcept TargetRegisters>
    void assign(const Source& source, SourceRegisters source_registers, const Target& target, TargetRegisters target_registers);

    /// Clear logical contents while retaining all owned buffers.
    void clear() noexcept
    {
        added.clear();
        removed.clear();
    }

    bool empty() const noexcept
    {
        return added.fluent_atoms.empty() && removed.fluent_atoms.empty() && added.derived_atoms.empty() && removed.derived_atoms.empty()
               && added.concept_registers.empty() && removed.concept_registers.empty() && added.role_registers.empty() && removed.role_registers.empty();
    }

    /// Undo uses the same payload with additions and removals exchanged.
    void reverse() noexcept { std::swap(added, removed); }
};

namespace detail
{
template<ConceptOrRoleTag Category, RegisterValuesViewConcept Source, RegisterValuesViewConcept Target, typename Value>
void assign_register_delta(Source source,
                           Target target,
                           std::vector<std::pair<RegisterIdentifier<Category>, Value>>& removed,
                           std::vector<std::pair<RegisterIdentifier<Category>, Value>>& added)
{
    const auto before = source.template get<Category>();
    const auto after = target.template get<Category>();
    const auto snapshot = [](const auto& values, size_t i) -> std::optional<Value>
    {
        if (i >= values.size() || !values[i])
            return std::nullopt;
        const auto value = values[i].value();
        if constexpr (std::same_as<Category, ConceptTag>)
            return value;
        else
            return std::pair(value.get_first(), value.get_second());
    };
    for (size_t i = 0; i < std::max(before.size(), after.size()); ++i)
    {
        const auto old_value = snapshot(before, i);
        const auto new_value = snapshot(after, i);
        if (old_value == new_value)
            continue;
        const auto identifier = RegisterIdentifier<Category>(static_cast<ygg::uint_t>(i));
        if (old_value)
            removed.emplace_back(identifier, *old_value);
        if (new_value)
            added.emplace_back(identifier, *new_value);
    }
}
}  // namespace detail

template<tyr::TaskKind Kind,
         tyr::planning::StateViewConcept<Kind> Source,
         RegisterValuesViewConcept SourceRegisters,
         tyr::planning::StateViewConcept<Kind> Target,
         RegisterValuesViewConcept TargetRegisters>
void Delta<runir::kr::ExtFamilyTag>::assign(const Source& source, SourceRegisters source_registers, const Target& target, TargetRegisters target_registers)
{
    const auto* repository = source.get_formalism_repository().get();
    if (&source_registers.get_formalism_repository() != repository || &target_registers.get_formalism_repository() != repository)
        throw std::invalid_argument("Feature delta requires registers for the same planning task.");

    detail::assign_atom_delta<Kind>(source, target, *this);

    detail::assign_register_delta<ConceptTag>(source_registers, target_registers, removed.concept_registers, added.concept_registers);
    detail::assign_register_delta<RoleTag>(source_registers, target_registers, removed.role_registers, added.role_registers);
}

}  // namespace runir::kr::dl::semantics::incremental

#endif
