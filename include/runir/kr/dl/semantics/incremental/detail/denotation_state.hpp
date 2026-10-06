#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_DENOTATION_STATE_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_DENOTATION_STATE_HPP_

#include "runir/kr/dl/semantics/incremental/denotation_delta.hpp"
#include "runir/kr/dl/semantics/interning.hpp"

namespace runir::kr::dl::semantics::incremental::detail
{

/// Retained storage shared by constructor-specific update operations.
/// Set each affected element to its final membership, avoiding transient deltas.
template<CategoryTag Category>
class DenotationState;

template<ConceptOrRoleTag Category>
class DenotationState<Category>
{
    ygg::Builder<Denotation<Category>> m_result;
    DenotationDelta<Category> m_delta;

public:
    /// Initialize storage and return its builder for construction without recording changes.
    auto& initialize(ygg::uint_t num_objects)
    {
        m_result.initialize(num_objects);
        m_delta.clear();
        return m_result;
    }

    void clear_delta() noexcept { m_delta.clear(); }
    const auto& get_builder() const noexcept { return m_result; }
    const auto& get_delta() const noexcept { return m_delta; }
    size_t size() const noexcept { return m_result.count(); }

    /// Copy a complete result while retaining storage and clearing previous changes.
    auto& assign(DenotationView<Category> source)
    {
        semantics::assign(m_result, source);
        m_delta.clear();
        return m_result;
    }

    auto& assign(BorrowedDenotationView<Category> source)
    {
        semantics::assign(m_result, source);
        m_delta.clear();
        return m_result;
    }

    bool contains(DenotationElementView<Category> value) const noexcept
    {
        if constexpr (std::same_as<Category, ConceptTag>)
            return m_result.contains(value.get_index());
        else
            return m_result.contains(value.first.get_index(), value.second.get_index());
    }

    /// Update membership and record its change exactly once.
    void set(DenotationElementView<Category> value, bool present)
    {
        if (contains(value) == present)
            return;
        (present ? m_delta.added : m_delta.removed).push_back(value);
        if constexpr (std::same_as<Category, ConceptTag>)
            m_result.set(value.get_index(), present);
        else
            m_result.set(value.first.get_index(), value.second.get_index(), present);
    }

    /// Internal computations use indices; construct views only for recorded changes.
    void set(ygg::Index<tyr::formalism::Object> object, bool present, const tyr::formalism::planning::Repository& repository)
        requires std::same_as<Category, ConceptTag>
    {
        if (m_result.contains(object) != present)
            set(ygg::make_view(object, repository), present);
    }

    void set(ygg::Index<tyr::formalism::Object> source,
             ygg::Index<tyr::formalism::Object> target,
             bool present,
             const tyr::formalism::planning::Repository& repository)
        requires std::same_as<Category, RoleTag>
    {
        if (m_result.contains(source, target) != present)
            set(std::pair(ygg::make_view(source, repository), ygg::make_view(target, repository)), present);
    }

    /// Row computations produce a complete row; dependent evaluators need only changed pairs.
    /// Compare old and new membership before overwriting it, recording the exact delta.
    void update_row(ygg::Index<tyr::formalism::Object> source, ygg::BitsetSpan<const ygg::uint_t> row, const tyr::formalism::planning::Repository& repository)
        requires std::same_as<Category, RoleTag>
    {
        // Each XOR word is captured before its changed memberships are updated.
        ygg::for_each_bit(
            [&](size_t target)
            {
                const auto object = ygg::Index<tyr::formalism::Object>(static_cast<ygg::uint_t>(target));
                set(source, object, row.test(static_cast<ygg::uint_t>(target)), repository);
            },
            [](auto before, auto after) { return before ^ after; },
            m_result.get(source),
            row);
    }

    BorrowedDenotationView<Category> get_result(const tyr::formalism::planning::Repository& repository) const noexcept
    {
        return ygg::make_view(m_result, repository);
    }
};

template<BooleanOrNumericalTag Category>
class DenotationState<Category>
{
    ygg::Builder<Denotation<Category>> m_result;
    bool m_changed = false;

public:
    void initialize(DenotationElementView<Category> value) noexcept
    {
        m_result.initialize(value);
        clear_delta();
    }
    void clear_delta() noexcept { m_changed = false; }
    const auto& get_builder() const noexcept { return m_result; }
    auto get_value() const noexcept { return m_result.get(); }
    bool changed() const noexcept { return m_changed; }
    void assign(DenotationView<Category> source) { initialize(source.get()); }
    void assign(BorrowedDenotationView<Category> source) { initialize(source.get()); }
    void set(DenotationElementView<Category> value) noexcept
    {
        m_changed = m_result.get() != value;
        m_result.get() = value;
    }
    BorrowedDenotationView<Category> get_result(const tyr::formalism::planning::Repository& repository) const noexcept
    {
        return ygg::make_view(m_result, repository);
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
