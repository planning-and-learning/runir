#ifndef RUNIR_KR_DL_CNF_GRAMMAR_PRUNING_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_PRUNING_HPP_

#include "generation_state.hpp"
#include "runir/kr/dl/cnf_grammar/declarations.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/evaluation_storage.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <cstddef>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_repository.hpp>
#include <tyr/planning/state_view.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>

namespace runir::kr::dl::cnf_grammar::detail
{

template<tyr::TaskKind Kind>
class Pruning
{
private:
    const urd::Store<urd::sequence<tyr::planning::PackedStateView<Kind>>>* m_states;
    runir::kr::dl::semantics::Builder m_builder;
    runir::kr::dl::semantics::DenotationRepository* m_denotation_repository;
    urd::Store<urd::sequence<runir::kr::dl::semantics::EvaluationStorage>, urd::stable_refs> m_intermediate_storage;
    urd::Store<urd::sequence<runir::kr::dl::semantics::DenotationCaches>> m_denotation_caches;
    SeenDenotations m_seen_denotations;

public:
    Pruning(const urd::Store<urd::sequence<tyr::planning::PackedStateView<Kind>>>& states,
            runir::kr::dl::semantics::DenotationRepository& denotation_repository) :
        m_states(&states),
        m_builder(),
        m_denotation_repository(&denotation_repository),
        m_intermediate_storage(),
        m_denotation_caches(states.size()),
        m_seen_denotations(states.size())
    {
        for (size_t i = 0; i < states.size(); ++i)
            m_intermediate_storage.emplace_back(denotation_repository);
    }

    template<runir::kr::dl::CategoryTag Category>
    bool should_prune(NonTerminalView<Category> lhs, runir::kr::dl::ConstructorView<Category> constructor)
    {
        auto& seen = m_seen_denotations.template get<Category>();
        if (m_states->empty())
            return !seen.constructors.insert(std::pair(lhs, constructor)).second;

        const auto was_cached = m_denotation_caches.front().find(constructor.get_is_static(), constructor).has_value();
        seen.scratch.clear();

        for (size_t i = 0; i < m_states->size(); ++i)
        {
            auto context = runir::kr::dl::semantics::StateEvaluationContext<Kind>((*m_states)[i].unpack(),
                                                                                  m_builder,
                                                                                  m_denotation_caches[i],
                                                                                  *m_denotation_repository,
                                                                                  m_intermediate_storage[i]);
            // Roots share one canonical repository across example states; their
            // intermediate results remain in the per-state reusable storage.
            seen.scratch.push_back(runir::kr::dl::semantics::evaluate<Kind>(constructor, context));
        }

        seen.scratch_indices.clear();
        for (const auto& denotation : seen.scratch)
            seen.scratch_indices.push_back(denotation.index());
        const auto vector = seen.vectors.insert(seen.scratch_indices).first.value;
        const auto is_novel = seen.signatures.insert(std::pair(lhs, vector)).second;

        if (!is_novel && !was_cached)
            for (auto& caches : m_denotation_caches)
                caches.erase(constructor.get_is_static(), constructor);

        return !is_novel;
    }
};

}

#endif
