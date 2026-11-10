#ifndef RUNIR_SEMANTICS_DENOTATION_VIEW_HPP_
#define RUNIR_SEMANTICS_DENOTATION_VIEW_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_builder.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"

#include <cassert>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/elements/view.hpp>
#include <urd/bits.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

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
using DenotationElementViewList = urd::Store<urd::sequence<DenotationElementView<Category>>, urd::contiguous>;

namespace detail
{
using DenotationBits = urd::bitset_span<const ygg::uint_t>;

/// The objects whose bits are set (a concept's bits, or a role's row): a borrowed range, holding only the bits.
inline auto object_indices(DenotationBits bits) noexcept
{
    return urd::transform(urd::set_bit_indices(bits),
                          [](std::size_t bit) { return urd::index<tyr::formalism::Object>(static_cast<ygg::uint_t>(bit)); });
}

/// The (source, target) pairs whose bits are set in a role's bits, a row of whole blocks per source.
inline auto object_pair_indices(DenotationBits bits, ygg::uint_t num_objects) noexcept
{
    return urd::transform(urd::set_bit_indices(bits),
                          [row_bits = static_cast<std::size_t>(DenotationBits::num_blocks(num_objects) * DenotationBits::Digits)](std::size_t bit)
                          {
                              return std::pair(urd::index<tyr::formalism::Object>(static_cast<ygg::uint_t>(bit / row_bits)),
                                               urd::index<tyr::formalism::Object>(static_cast<ygg::uint_t>(bit % row_bits)));
                          });
}
}  // namespace detail

}  // namespace runir::kr::dl::semantics

/// Borrows mutable denotation storage through a read-only interface. The builder
/// and formalism repository must outlive the view; mutations invalidate iterators.
template<runir::kr::dl::CategoryTag Category, typename V>
struct urd::view_mixin<urd::builder<runir::kr::dl::semantics::Denotation<Category>>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    const auto& get_formalism_repository() const noexcept { return self().context(); }

    auto get_num_objects() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return self().data().num_objects; }

    [[nodiscard]] bool any() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return storage_bits().any(); }

    [[nodiscard]] size_t count() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return storage_bits().count(); }

    // Internal interface: raw values, bitsets and typed indices avoid constructing object views.

    auto value() const noexcept
        requires(!std::same_as<Category, runir::kr::dl::RoleTag>)
    { return self().data().get(); }

    auto value(urd::index<tyr::formalism::Object> object) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return self().data().get(object); }

    /// Includes zero padding: trailing concept bits or padding after each role row.
    auto storage_bits() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return self().data().storage_bits(); }

    /// Typed indices borrowing only the bits, independently of the view and range wrappers.
    auto indices() const noexcept
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    { return runir::kr::dl::semantics::detail::object_indices(storage_bits()); }

    auto indices() const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return runir::kr::dl::semantics::detail::object_pair_indices(storage_bits(), get_num_objects()); }

    auto indices(urd::index<tyr::formalism::Object> source) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return runir::kr::dl::semantics::detail::object_indices(value(source)); }

    // External interface: object views bound to the formalism repository, borrowing the bits and the repository.

    auto views() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return urd::resolve(indices(), get_formalism_repository()); }

    /// Objects reached from this source.
    auto views(tyr::formalism::planning::ObjectView source) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return urd::resolve(indices(source.index()), get_formalism_repository()); }

    auto begin() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return views().begin(); }

    auto begin(tyr::formalism::planning::ObjectView source) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return views(source).begin(); }

    auto end() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return views().end(); }

    auto end(tyr::formalism::planning::ObjectView source) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return views(source).end(); }
};


namespace urd
{
template<runir::kr::dl::CategoryTag Category, typename V>
struct view_mixin<runir::kr::dl::semantics::Denotation<Category>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_vector() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag> || std::same_as<Category, runir::kr::dl::RoleTag>)
    { return self().context().get_vector_repository().at(self().data().vec_index); }

    const auto& get_formalism_repository() const noexcept { return self().context().get_formalism_repository(); }

    auto get_num_objects() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return self().data().num_objects; }

    [[nodiscard]] bool any() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return storage_bits().any(); }

    [[nodiscard]] size_t count() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return storage_bits().count(); }

    // Internal interface: raw values, bitsets and typed indices avoid constructing object views.

    auto value() const noexcept
        requires runir::kr::dl::BooleanOrNumericalTag<Category>
    { return self().data().get_data(); }

    auto value() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag>)
    {
        using Layout = urd::builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>;
        using Bitset = urd::bitset_span<const ygg::uint_t>;

        const auto vector = get_vector();
        const auto& data = self().data();

        assert(vector.size() == Layout::num_blocks(data.num_objects));

        return Bitset(vector.data(), Layout::num_bits(data.num_objects));
    }

    auto value(urd::index<::tyr::formalism::Object> object) const noexcept
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        using Layout = urd::builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>;
        using Bitset = urd::bitset_span<const ygg::uint_t>;

        const auto vector = get_vector();
        const auto& data = self().data();

        assert(vector.size() == Layout::num_blocks(data.num_objects));
        assert(ygg::uint_t(object.value) < data.num_objects);

        return Bitset(vector.data() + Layout::row_block_offset(object, data.num_objects), data.num_objects);
    }

    /// Includes zero padding: trailing concept bits or padding after each role row.
    auto storage_bits() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    {
        using Bitset = urd::bitset_span<const ygg::uint_t>;
        const auto vector = get_vector();
        return Bitset(vector.data(), vector.size() * Bitset::Digits);
    }

    /// Typed indices borrowing only the bits, independently of the view and range wrappers.
    auto indices() const noexcept
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    { return runir::kr::dl::semantics::detail::object_indices(storage_bits()); }

    auto indices() const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return runir::kr::dl::semantics::detail::object_pair_indices(storage_bits(), get_num_objects()); }

    auto indices(urd::index<tyr::formalism::Object> source) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return runir::kr::dl::semantics::detail::object_indices(value(source)); }

    // External interface: object views bound to the formalism repository, borrowing the bits and the repository.

    auto views() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return urd::resolve(indices(), get_formalism_repository()); }

    /// Objects reached from this source.
    auto views(tyr::formalism::planning::ObjectView source) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return urd::resolve(indices(source.index()), get_formalism_repository()); }

    auto begin() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return views().begin(); }

    auto begin(tyr::formalism::planning::ObjectView source) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return views(source).begin(); }

    auto end() const noexcept
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return views().end(); }

    auto end(tyr::formalism::planning::ObjectView source) const noexcept
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return views(source).end(); }
};
}  // namespace urd

namespace runir::kr::dl::semantics
{

/// The interned and borrowed representations share the denotation interface for this category.
template<typename V, typename Category>
concept DenotationViewConcept =
    CategoryTag<Category>
    && (std::same_as<std::remove_cvref_t<V>, DenotationView<Category>> || std::same_as<std::remove_cvref_t<V>, BorrowedDenotationView<Category>>);

}  // namespace runir::kr::dl::semantics

#endif
