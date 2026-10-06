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
    size_t m_size = 0;

public:
    void initialize(ygg::uint_t num_objects)
    {
        m_result.initialize(num_objects);
        m_delta.clear();
        m_size = 0;
    }

    void clear_delta() noexcept { m_delta.clear(); }
    const auto& get_builder() const noexcept { return m_result; }
    const auto& get_delta() const noexcept { return m_delta; }
    size_t size() const noexcept { return m_size; }

    /// Replace the baseline while retaining storage. No change is published.
    void assign(DenotationView<Category> source)
    {
        semantics::assign(m_result, source);
        m_size = source.count();
        m_delta.clear();
    }

    void assign(BorrowedDenotationView<Category> source)
    {
        if (&source.get_handle() != &m_result)
        {
            m_result.blocks = source.get_handle().blocks;
            m_result.num_objects = source.get_num_objects();
            ygg::clear(m_result.index);
        }
        m_size = source.count();
        m_delta.clear();
    }

    /// Complement a baseline; padding stays zero and no change is published.
    void flip() noexcept
    {
        if constexpr (std::same_as<Category, ConceptTag>)
        {
            m_result.get().flip();
            m_size = m_result.num_objects - m_size;
        }
        else
        {
            for (ygg::uint_t source = 0; source < m_result.num_objects; ++source)
                m_result.get(source).flip();
            m_size = static_cast<size_t>(m_result.num_objects) * m_result.num_objects - m_size;
        }
        m_delta.clear();
    }

    bool contains(DenotationElementView<Category> value) const noexcept
    {
        if constexpr (std::same_as<Category, ConceptTag>)
            return m_result.get().test(ygg::uint_t(value.get_index()));
        else
            return m_result.get(value.first.get_index()).test(ygg::uint_t(value.second.get_index()));
    }

    void set(DenotationElementView<Category> value, bool present)
    {
        if (contains(value) == present)
            return;
        (present ? m_delta.added : m_delta.removed).push_back(value);
        if constexpr (std::same_as<Category, ConceptTag>)
            m_result.get().set(ygg::uint_t(value.get_index()), present);
        else
            m_result.get(value.first.get_index()).set(ygg::uint_t(value.second.get_index()), present);
        if (present)
            ++m_size;
        else
            --m_size;
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
