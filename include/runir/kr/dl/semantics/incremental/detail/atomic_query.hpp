#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_ATOMIC_QUERY_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_ATOMIC_QUERY_HPP_

#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/ext/evaluation.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <tyr/planning/state_view.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/database/incremental/delta.hpp>
#include <yggdrasil/database/incremental/join.hpp>
#include <yggdrasil/database/incremental/projection.hpp>

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
    tyr::formalism::planning::PredicateView<Fact> m_predicate;
    bool m_initialized = false;
    ygg::Builder<ygg::database::Relation<ygg::Index<tyr::formalism::Object>>> m_result;
    ygg::database::incremental::Delta<ygg::Index<tyr::formalism::Object>> m_delta;

public:
    template<FamilyTag Family, typename C>
    explicit AtomicQueryEvaluator(ygg::View<ygg::Index<Query<Family, AtomicStateTag<Fact>>>, C> query);

    /// Replace the baseline from a completed state, retaining buffers and clearing the last delta.
    template<tyr::TaskKind Kind, tyr::planning::StateViewConcept<Kind> State>
    void initialize(const State& state);

    /// Actual, disjoint atom changes from the initialized task. Other predicates are ignored.
    /// Additions must be absent and removals present in the current result.
    void update(std::span<const tyr::formalism::planning::AtomView<tyr::GroundTag, Fact>> added,
                std::span<const tyr::formalism::planning::AtomView<tyr::GroundTag, Fact>> removed);

    const ygg::Builder<ygg::database::Relation<ygg::Index<tyr::formalism::Object>>>& get_result() const& noexcept { return m_result; }
    const ygg::Builder<ygg::database::Relation<ygg::Index<tyr::formalism::Object>>>& get_result() const&& = delete;
    const ygg::database::incremental::Delta<ygg::Index<tyr::formalism::Object>>& get_delta() const& noexcept { return m_delta; }
    const ygg::database::incremental::Delta<ygg::Index<tyr::formalism::Object>>& get_delta() const&& = delete;
};

template<tyr::formalism::FactKind Fact>
    requires(std::same_as<Fact, tyr::formalism::FluentTag> || std::same_as<Fact, tyr::formalism::DerivedTag>)
template<FamilyTag Family, typename C>
AtomicQueryEvaluator<Fact>::AtomicQueryEvaluator(ygg::View<ygg::Index<Query<Family, AtomicStateTag<Fact>>>, C> query) :
    m_predicate(query.get_predicate()),
    m_result(query.get_schema().span()),
    m_delta(query.get_schema().span())
{
}

template<tyr::formalism::FactKind Fact>
    requires(std::same_as<Fact, tyr::formalism::FluentTag> || std::same_as<Fact, tyr::formalism::DerivedTag>)
template<tyr::TaskKind Kind, tyr::planning::StateViewConcept<Kind> State>
void AtomicQueryEvaluator<Fact>::initialize(const State& state)
{
    if (!state.get_repository()->contains(m_predicate))
        throw std::invalid_argument("Incremental atomic query: predicate does not belong to the task repository.");
    m_initialized = false;
    m_result.clear();
    m_delta.clear();
    for (const auto atom : state.get_atoms_view(m_predicate))
        m_result.insert(atom.get_row().get_data());
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
        const auto& row = atom.get_row().get_data();
        if (m_result.contains(row))
            throw std::invalid_argument("Incremental atomic query: added atom is already present.");
        m_delta.added.insert(row);
    }
    for (const auto atom : removed)
    {
        if (atom.get_predicate() != m_predicate)
            continue;
        const auto& row = atom.get_row().get_data();
        const auto position = m_result.find(row);
        if (!position)
            throw std::invalid_argument("Incremental atomic query: removed atom is absent.");
        m_delta.removed.insert(row);
        m_result.erase(*position);
    }
    for (size_t i = 0; i < m_delta.added.size(); ++i)
        m_result.insert(m_delta.added.row(i));
    m_initialized = true;
}

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
