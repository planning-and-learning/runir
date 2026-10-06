#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_REGISTER_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_REGISTER_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"

#include <concepts>
#include <stdexcept>
#include <utility>
#include <vector>

namespace runir::kr::dl::semantics::incremental::detail
{

template<FamilyTag Family, tyr::TaskKind Kind, CategoryTag Category>
struct RegisterEvaluator
{
    RegisterIdentifier<Category> identifier;
    DenotationState<Category> value;

    explicit RegisterEvaluator(RegisterIdentifier<Category> identifier) : identifier(identifier) {}
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>&, Context& context)
    {
        auto& builder = value.initialize(semantics::detail::num_objects<Kind, Family>(context));
        const auto slot = context.registers().at(identifier);
        if (slot)
        {
            if constexpr (std::same_as<Category, ConceptTag>)
                builder.set(slot.value().get_index(), true);
            else
                builder.set(slot.value().get_first().get_index(), slot.value().get_second().get_index(), true);
        }
    }
    void update(EvaluationGraph<Family, Kind>&, const Delta<Family>& delta, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
    {
        if constexpr (std::same_as<Category, ConceptTag>)
            update_values(delta.added.concept_registers, delta.removed.concept_registers);
        else
            update_values(delta.added.role_registers, delta.removed.role_registers);
    }

private:
    void update_values(const std::vector<std::pair<RegisterIdentifier<Category>, DenotationElementView<Category>>>& added,
                       const std::vector<std::pair<RegisterIdentifier<Category>, DenotationElementView<Category>>>& removed)
    {
        for (const auto& [slot, element] : added)
            if (slot == identifier && value.contains(element))
                throw std::invalid_argument("Incremental DL: added register value is already present.");
        for (const auto& [slot, element] : removed)
            if (slot == identifier)
            {
                if (!value.contains(element))
                    throw std::invalid_argument("Incremental DL: removed register value is absent.");
                value.set(element, false);
            }
        for (const auto& [slot, element] : added)
            if (slot == identifier)
                value.set(element, true);
        // A valid register can exceed one member only after an addition.
        if (!value.get_delta().added.empty() && value.size() > 1)
            throw std::invalid_argument("Incremental DL: a register can hold at most one value.");
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
