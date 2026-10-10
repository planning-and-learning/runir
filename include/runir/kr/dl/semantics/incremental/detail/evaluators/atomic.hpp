#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_ATOMIC_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_ATOMIC_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"

#include <concepts>
#include <span>
#include <stdexcept>
#include <utility>

namespace runir::kr::dl::semantics::incremental::detail
{

template<FamilyTag Family, tyr::TaskKind Kind, CategoryTag Category, tyr::formalism::FactKind Fact>
struct AtomicEvaluator
{
    tyr::formalism::planning::PredicateView<Fact> predicate;
    bool polarity;
    DenotationState<Category> value;

    AtomicEvaluator(tyr::formalism::planning::PredicateView<Fact> predicate, bool polarity) : predicate(predicate), polarity(polarity) {}
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context& context)
    {
        if (!graph.repository().contains(predicate))
            throw std::invalid_argument("Incremental DL: predicate does not belong to the task repository.");
        if constexpr (std::same_as<Category, BooleanTag>)
        {
            bool present = false;
            for ([[maybe_unused]] const auto atom : context.get_state().get_atoms_view(predicate))
                present = true;
            value.set(present == polarity);
        }
        else
        {
            auto& builder = value.initialize(semantics::detail::num_objects<Kind, Family>(context));
            for (const auto atom : context.get_state().get_atoms_view(predicate))
            {
                const auto objects = atom.get_row().get_objects();
                if constexpr (std::same_as<Category, ConceptTag>)
                    builder.set(objects[0].get_index(), true);
                else
                    builder.set(objects[0].get_index(), objects[1].get_index(), true);
            }
            if (!polarity)
                builder.flip();
        }
    }
    void update(EvaluationGraph<Family, Kind>&, const Delta<Family>& delta, ygg::database::Workspace<ObjectValues>&)
    {
        if constexpr (std::same_as<Fact, tyr::formalism::FluentTag>)
            update_atoms(delta.added.fluent_atoms, delta.removed.fluent_atoms);
        else
            update_atoms(delta.added.derived_atoms, delta.removed.derived_atoms);
    }

private:
    static DenotationElementView<Category> atom_element(tyr::formalism::planning::AtomView<tyr::GroundTag, Fact> atom)
        requires ConceptOrRoleTag<Category>
    {
        const auto objects = atom.get_row().get_objects();
        if constexpr (std::same_as<Category, ConceptTag>)
            return objects[0];
        else
            return std::pair(objects[0], objects[1]);
    }

    void update_atoms(std::span<const tyr::formalism::planning::AtomView<tyr::GroundTag, Fact>> added,
                      std::span<const tyr::formalism::planning::AtomView<tyr::GroundTag, Fact>> removed)
    {
        const auto present = [&](auto atom)
        {
            if constexpr (std::same_as<Category, BooleanTag>)
                return value.get_value() == polarity;
            else
                return value.contains(atom_element(atom)) == polarity;
        };
        const auto change = [&](auto atom, bool present)
        {
            if constexpr (std::same_as<Category, BooleanTag>)
                value.set(present == polarity);
            else
                value.set(atom_element(atom), present == polarity);
        };
        for (const auto atom : added)
            if (atom.get_predicate() == predicate && present(atom))
                throw std::invalid_argument("Incremental DL: added atom is already present.");
        for (const auto atom : removed)
            if (atom.get_predicate() == predicate)
            {
                if (!present(atom))
                    throw std::invalid_argument("Incremental DL: removed atom is absent.");
                change(atom, false);
            }
        for (const auto atom : added)
            if (atom.get_predicate() == predicate)
                change(atom, true);
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
