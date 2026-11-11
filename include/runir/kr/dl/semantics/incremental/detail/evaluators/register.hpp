#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_REGISTER_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_REGISTER_HPP_

#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/detail/evaluation_helpers.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <concepts>
#include <stdexcept>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>

namespace runir::kr::dl::semantics::incremental::detail
{

template<tyr::TaskKind Kind, CategoryTag Category>
struct RegisterEvaluator
{
    RegisterIdentifier<Category> identifier;
    DenotationState<Category> value;

    explicit RegisterEvaluator(RegisterIdentifier<Category> identifier_) : identifier(identifier_) {}
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>&, Context& context)
    {
        auto& builder = value.initialize(semantics::detail::num_objects<Kind>(context));
        const auto slot = register_value(context, identifier);
        if (slot)
        {
            if constexpr (std::same_as<Category, ConceptTag>)
                builder.set(slot.value().index(), true);
            else
                builder.set(slot.value().first.index(), slot.value().second.index(), true);
        }
    }
    void update(EvaluationGraph<Kind>&, const Delta& delta, tyr::datalog::database::Workspace&)
    {
        if constexpr (std::same_as<Category, ConceptTag>)
            update_values(delta.added.concept_registers, delta.removed.concept_registers);
        else
            update_values(delta.added.role_registers, delta.removed.role_registers);
    }

private:
    void update_values(const urd::Store<urd::sequence<std::pair<RegisterIdentifier<Category>, DenotationElementView<Category>>>>& added,
                       const urd::Store<urd::sequence<std::pair<RegisterIdentifier<Category>, DenotationElementView<Category>>>>& removed)
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
