#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_ATOMIC_QUERY_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_ATOMIC_QUERY_HPP_

#include "runir/kr/dl/object_rows.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <tyr/planning/state_view_concepts.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/database/semantics/incremental/delta.hpp>
#include <yggdrasil/database/semantics/incremental/join.hpp>
#include <yggdrasil/database/semantics/incremental/projection.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

/// Maintains a dynamic predicate's complete tuples without repository publication.
/// The planning repository must outlive this evaluator. Results and row deltas
/// are borrowed until the next mutation; consume the delta before updating again.
/// Reinitialize dynamic evaluators on invocation changes, retaining task-static results.
/// After an initialization or update fails during mutation, initialize before reuse.
template<tyr::formalism::FactKind Fact>
    requires(std::same_as<Fact, tyr::formalism::FluentTag> || std::same_as<Fact, tyr::formalism::DerivedTag>)
class AtomicQueryEvaluator
{
    const ygg::database::Repository* m_layer;
    tyr::formalism::planning::PredicateView<Fact> m_predicate;
    bool m_initialized = false;
    urd::builder<ygg::Relation<ygg::Schema>> m_result;
    ygg::database::incremental::Delta m_delta;
    urd::Store<urd::sequence<ygg::Value>> m_row;

public:
    /// Its builders are read through `layer`, which must outlive it.
    template<typename C>
    AtomicQueryEvaluator(const ygg::database::Repository& layer, urd::index_view<Query<AtomicStateTag<Fact>>, C> query);

    /// Replace the baseline from a completed state, retaining buffers and clearing the last delta.
    template<tyr::TaskKind Kind, tyr::planning::StateViewConcept<Kind> State>
    void initialize(const State& state);

    /// Actual, disjoint atom changes from the initialized task. Other predicates are ignored.
    /// Additions must be absent and removals present in the current result.
    void update(std::span<const tyr::formalism::planning::AtomView<tyr::GroundTag, Fact>> added,
                std::span<const tyr::formalism::planning::AtomView<tyr::GroundTag, Fact>> removed);

    [[nodiscard]] ygg::database::BuilderRelationView get_result() const& noexcept { return m_layer->view(m_result); }
    [[nodiscard]] ygg::database::BuilderRelationView get_result() const&& = delete;
    [[nodiscard]] const ygg::database::incremental::Delta& get_delta() const& noexcept { return m_delta; }
    [[nodiscard]] const ygg::database::incremental::Delta& get_delta() const&& = delete;
};

template<tyr::formalism::FactKind Fact>
    requires(std::same_as<Fact, tyr::formalism::FluentTag> || std::same_as<Fact, tyr::formalism::DerivedTag>)
template<typename C>
AtomicQueryEvaluator<Fact>::AtomicQueryEvaluator(const ygg::database::Repository& layer, urd::index_view<Query<AtomicStateTag<Fact>>, C> query) :
    m_layer(&layer),
    m_predicate(query.get_predicate()),
    m_result(query.get_schema()),
    m_delta(layer, query.get_schema())
{
}

template<tyr::formalism::FactKind Fact>
    requires(std::same_as<Fact, tyr::formalism::FluentTag> || std::same_as<Fact, tyr::formalism::DerivedTag>)
template<tyr::TaskKind Kind, tyr::planning::StateViewConcept<Kind> State>
void AtomicQueryEvaluator<Fact>::initialize(const State& state)
{
    if (!ygg::formalism::contains(*state.get_formalism_repository(), m_predicate))
        throw std::invalid_argument("Incremental atomic query: predicate does not belong to the task repository.");
    m_initialized = false;
    m_result.clear();
    m_delta.clear();
    for (const auto atom : state.get_atoms_view(m_predicate))
        m_result.insert(encode_object_row(atom.get_row().get_key().second, m_row));
    m_initialized = true;
}

template<tyr::formalism::FactKind Fact>
    requires(std::same_as<Fact, tyr::formalism::FluentTag> || std::same_as<Fact, tyr::formalism::DerivedTag>)
void AtomicQueryEvaluator<Fact>::update(std::span<const tyr::formalism::planning::AtomView<tyr::GroundTag, Fact>> added,
                                        std::span<const tyr::formalism::planning::AtomView<tyr::GroundTag, Fact>> removed)
{
    if (!std::exchange(m_initialized, false))
        throw std::logic_error("Incremental atomic query: initialize before updating.");
    m_delta.clear();

    // Check additions against the original result, so an atom cannot occur on both sides.
    for (const auto atom : added)
    {
        if (atom.get_predicate() != m_predicate)
            continue;
        const auto row = encode_object_row(atom.get_row().get_key().second, m_row);
        if (m_result.contains(row))
            throw std::invalid_argument("Incremental atomic query: added atom is already present.");
        m_delta.added.insert(row);
    }
    for (const auto atom : removed)
    {
        if (atom.get_predicate() != m_predicate)
            continue;
        const auto row = encode_object_row(atom.get_row().get_key().second, m_row);
        const auto position = m_result.find(row);
        if (!position)
            throw std::invalid_argument("Incremental atomic query: removed atom is absent.");
        m_delta.removed.insert(row);
        m_result.erase(*position);
    }
    const auto added_rows = m_layer->view(m_delta.added);
    for (size_t i = 0; i < added_rows.size(); ++i)
        m_result.insert(added_rows.row(i));
    m_initialized = true;
}

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
