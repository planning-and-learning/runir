#ifndef RUNIR_SEMANTICS_DENOTATION_VIEW_HPP_
#define RUNIR_SEMANTICS_DENOTATION_VIEW_HPP_

#include "runir/kr/dl/semantics/denotation_builder.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"
#include "runir/kr/dl/semantics/denotation_index.hpp"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <tuple>
#include <tyr/formalism/object_index.hpp>
#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/containers/dynamic_bitset.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::semantics
{

template<runir::kr::dl::CategoryTag Category>
struct DenotationElementType;

template<>
struct DenotationElementType<runir::kr::dl::BooleanTag>
{
    using Type = bool;
};

template<>
struct DenotationElementType<runir::kr::dl::NumericalTag>
{
    using Type = ygg::uint_t;
};

template<>
struct DenotationElementType<runir::kr::dl::ConceptTag>
{
    using Type = tyr::formalism::planning::ObjectView;
};

template<>
struct DenotationElementType<runir::kr::dl::RoleTag>
{
    using Type = std::pair<tyr::formalism::planning::ObjectView, tyr::formalism::planning::ObjectView>;
};

template<runir::kr::dl::CategoryTag Category>
using DenotationElementView = typename DenotationElementType<Category>::Type;

template<runir::kr::dl::CategoryTag Category>
using DenotationElementViewList = std::vector<DenotationElementView<Category>>;

namespace detail
{
/// Borrows the bits and repository independently of the view object.
/// Comparing iterators requires that they belong to the same sequence.
template<ConceptOrRoleTag Category>
class DenotationIterator
{
    ygg::SetBitIndices<ygg::uint_t>::Iterator m_bit;
    const tyr::formalism::planning::Repository* m_repository = nullptr;
    size_t m_row_bits = 0;

public:
    using value_type = DenotationElementView<Category>;
    using difference_type = std::ptrdiff_t;
    using reference = value_type;
    using pointer = void;
    using iterator_category = std::forward_iterator_tag;
    using iterator_concept = std::forward_iterator_tag;

    DenotationIterator() noexcept = default;
    DenotationIterator(ygg::BitsetSpan<const ygg::uint_t> bits,
                       const tyr::formalism::planning::Repository& repository,
                       bool begin,
                       ygg::uint_t num_objects = 0) noexcept :
        m_bit(bits, begin ? bits.find_first() : ygg::BitsetSpan<const ygg::uint_t>::npos),
        m_repository(&repository),
        m_row_bits(ygg::BitsetSpan<const ygg::uint_t>::num_blocks(num_objects) * ygg::BitsetSpan<const ygg::uint_t>::Digits)
    {
    }

    reference operator*() const noexcept
    {
        if constexpr (std::same_as<Category, ConceptTag>)
            return ygg::make_view(ygg::Index<tyr::formalism::Object>(static_cast<ygg::uint_t>(*m_bit)), *m_repository);
        else
            return std::pair(ygg::make_view(ygg::Index<tyr::formalism::Object>(static_cast<ygg::uint_t>(*m_bit / m_row_bits)), *m_repository),
                             ygg::make_view(ygg::Index<tyr::formalism::Object>(static_cast<ygg::uint_t>(*m_bit % m_row_bits)), *m_repository));
    }

    DenotationIterator& operator++() noexcept
    {
        ++m_bit;
        return *this;
    }
    DenotationIterator operator++(int) noexcept
    {
        auto previous = *this;
        ++*this;
        return previous;
    }
    friend bool operator==(const DenotationIterator& lhs, const DenotationIterator& rhs) noexcept { return lhs.m_bit == rhs.m_bit; }
};
}  // namespace detail

}  // namespace runir::kr::dl::semantics

namespace ygg
{

/// Borrows mutable denotation storage through a read-only interface. The builder
/// and formalism repository must outlive the view; mutations invalidate iterators.
template<runir::kr::dl::CategoryTag Category, typename C>
class View<Builder<runir::kr::dl::semantics::Denotation<Category>>, C>
{
    const Builder<runir::kr::dl::semantics::Denotation<Category>>* m_handle;
    const C* m_context;

public:
    View(const Builder<runir::kr::dl::semantics::Denotation<Category>>& handle, const C& context) noexcept : m_handle(&handle), m_context(&context) {}

    const auto& get_handle() const noexcept { return *m_handle; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_formalism_repository() const noexcept { return *m_context; }

    auto get() const noexcept
        requires(!std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        return m_handle->get();
    }

    auto get(Index<tyr::formalism::Object> object) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        return m_handle->get(object);
    }

    auto get(ygg::uint_t object) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        return get(Index<tyr::formalism::Object>(object));
    }

    auto begin(tyr::formalism::planning::ObjectView source) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        return runir::kr::dl::semantics::detail::DenotationIterator<runir::kr::dl::ConceptTag>(get(source.get_index()), get_formalism_repository(), true);
    }

    auto end(tyr::formalism::planning::ObjectView source) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        return runir::kr::dl::semantics::detail::DenotationIterator<runir::kr::dl::ConceptTag>(get(source.get_index()), get_formalism_repository(), false);
    }

    auto range() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    {
        return std::ranges::subrange(begin(), end());
    }

    /// Objects reached from this source, borrowing the row and repository.
    auto range(tyr::formalism::planning::ObjectView source) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        return std::ranges::subrange(begin(source), end(source));
    }

    auto get_num_objects() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    {
        return m_handle->num_objects;
    }

    /// Includes the zero padding at the end of every role row.
    auto storage_bits() const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        return m_handle->storage_bits();
    }

    bool any() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return get().any();
        else
            return storage_bits().any();
    }

    size_t count() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return get().count();
        else
            return storage_bits().count();
    }

    auto begin() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return runir::kr::dl::semantics::detail::DenotationIterator<Category>(get(), get_formalism_repository(), true);
        else
            return runir::kr::dl::semantics::detail::DenotationIterator<Category>(storage_bits(), get_formalism_repository(), true, get_num_objects());
    }

    auto end() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return runir::kr::dl::semantics::detail::DenotationIterator<Category>(get(), get_formalism_repository(), false);
        else
            return runir::kr::dl::semantics::detail::DenotationIterator<Category>(storage_bits(), get_formalism_repository(), false, get_num_objects());
    }
};

template<runir::kr::dl::CategoryTag Category, typename C>
class View<Index<runir::kr::dl::semantics::Denotation<Category>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::dl::semantics::Denotation<Category>> m_handle;

    auto get_vector() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag> || std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        return get_denotation_vector_repository(*m_context)[get_data().vec_index];
    }

public:
    View(Index<runir::kr::dl::semantics::Denotation<Category>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_denotation_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    const auto& get_formalism_repository() const noexcept { return get_denotation_repository(*m_context).get_formalism_repository(); }

    auto get() const noexcept
        requires runir::kr::dl::BooleanOrNumericalTag<Category>
    {
        return get_data().get_data();
    }

    auto get() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag>)
    {
        using Layout = Builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>;
        using Bitset = BitsetSpan<const ygg::uint_t>;

        const auto vector = get_vector();
        const auto& data = get_data();

        assert(vector.size() == Layout::num_blocks(data.num_objects));

        return Bitset(vector.data(), Layout::num_bits(data.num_objects));
    }

    auto get(Index<::tyr::formalism::Object> object) const noexcept
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        using Layout = Builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>;
        using Bitset = BitsetSpan<const ygg::uint_t>;

        const auto vector = get_vector();
        const auto& data = get_data();

        assert(vector.size() == Layout::num_blocks(data.num_objects));
        assert(ygg::uint_t(object) < data.num_objects);

        return Bitset(vector.data() + Layout::row_block_offset(object, data.num_objects), data.num_objects);
    }

    auto get(ygg::uint_t object) const noexcept
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        return get(Index<::tyr::formalism::Object>(object));
    }

    auto begin(tyr::formalism::planning::ObjectView source) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        return runir::kr::dl::semantics::detail::DenotationIterator<runir::kr::dl::ConceptTag>(get(source.get_index()), get_formalism_repository(), true);
    }

    auto end(tyr::formalism::planning::ObjectView source) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        return runir::kr::dl::semantics::detail::DenotationIterator<runir::kr::dl::ConceptTag>(get(source.get_index()), get_formalism_repository(), false);
    }

    auto range() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    {
        return std::ranges::subrange(begin(), end());
    }

    /// Objects reached from this source, borrowing the row and repository.
    auto range(tyr::formalism::planning::ObjectView source) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        return std::ranges::subrange(begin(source), end(source));
    }

    auto get_num_objects() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    {
        return get_data().num_objects;
    }

    /// Includes the zero padding at the end of every role row.
    auto storage_bits() const noexcept
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        using Bitset = BitsetSpan<const ygg::uint_t>;
        const auto vector = get_vector();
        return Bitset(vector.data(), vector.size() * Bitset::Digits);
    }

    bool any() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return get().any();
        else
            return storage_bits().any();
    }

    size_t count() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return get().count();
        else
            return storage_bits().count();
    }

    auto begin() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return runir::kr::dl::semantics::detail::DenotationIterator<Category>(get(), get_formalism_repository(), true);
        else
            return runir::kr::dl::semantics::detail::DenotationIterator<Category>(storage_bits(), get_formalism_repository(), true, get_num_objects());
    }

    auto end() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return runir::kr::dl::semantics::detail::DenotationIterator<Category>(get(), get_formalism_repository(), false);
        else
            return runir::kr::dl::semantics::detail::DenotationIterator<Category>(storage_bits(), get_formalism_repository(), false, get_num_objects());
    }

    auto identifying_members() const noexcept { return std::make_tuple(m_handle, get_denotation_repository(*m_context).get_index()); }
};

}

#endif
