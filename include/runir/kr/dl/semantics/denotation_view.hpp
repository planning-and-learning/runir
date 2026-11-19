#ifndef RUNIR_SEMANTICS_DENOTATION_VIEW_HPP_
#define RUNIR_SEMANTICS_DENOTATION_VIEW_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_builder.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/bits.hpp>
#include <urd/elements/flat.hpp>
#include <urd/elements/view.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>
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
    return urd::transform(urd::set_bit_indices(bits), [](std::size_t bit) { return urd::index<tyr::formalism::Object>(static_cast<ygg::uint_t>(bit)); });
}

/// The (source, target) pairs whose bits are set in a role's bits, a row of whole blocks per source.
inline auto object_pair_indices(DenotationBits bits, ygg::uint_t num_objects) noexcept
{
    return urd::transform(urd::set_bit_indices(bits),
                          static_cast<std::size_t>(DenotationBits::num_blocks(num_objects) * DenotationBits::Digits),
                          [](std::size_t row_bits, std::size_t bit)
                          {
                              return std::pair(urd::index<tyr::formalism::Object>(static_cast<ygg::uint_t>(bit / row_bits)),
                                               urd::index<tyr::formalism::Object>(static_cast<ygg::uint_t>(bit % row_bits)));
                          });
}
}  // namespace detail

}  // namespace runir::kr::dl::semantics

namespace urd
{
template<runir::kr::dl::CategoryTag Category, typename V>
struct view_mixin<runir::kr::dl::semantics::Denotation<Category>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_value() const
        requires runir::kr::dl::BooleanOrNumericalTag<Category>
    { return self().template member<&urd::record_of<V>::value>(); }

    auto get_num_objects() const
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return self().template member<&urd::record_of<V>::num_objects>(); }

    /// Its bitset's words, read from its interned blocks.
    auto get_blocks() const
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return runir::kr::dl::semantics::detail::words(self().template member<&urd::record_of<V>::blocks>()); }

    // Internal interface: raw values, bitsets and typed indices avoid constructing object views.

    auto value() const
        requires(std::same_as<Category, runir::kr::dl::ConceptTag>)
    {
        using Layout = urd::builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>, runir::kr::dl::semantics::DenotationRepository>;
        using Bitset = urd::bitset_span<const ygg::uint_t>;

        const auto vector = get_blocks();
        assert(vector.size() == Layout::num_blocks(get_num_objects()));
        return Bitset(vector.data(), Layout::num_bits(get_num_objects()));
    }

    auto value(urd::index<::tyr::formalism::Object> object) const
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        using Layout = urd::builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>, runir::kr::dl::semantics::DenotationRepository>;
        using Bitset = urd::bitset_span<const ygg::uint_t>;

        const auto vector = get_blocks();
        assert(vector.size() == Layout::num_blocks(get_num_objects()));
        assert(ygg::uint_t(object.value) < get_num_objects());
        return Bitset(vector.data() + Layout::row_block_offset(object, get_num_objects()), get_num_objects());
    }

    /// Includes zero padding: trailing concept bits or padding after each role row.
    auto storage_bits() const
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    {
        using Bitset = urd::bitset_span<const ygg::uint_t>;
        const auto vector = get_blocks();
        return Bitset(vector.data(), vector.size() * Bitset::Digits);
    }

    /// Typed indices borrowing only the bits, independently of the view and range wrappers.
    auto indices() const
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    { return runir::kr::dl::semantics::detail::object_indices(storage_bits()); }

    auto indices() const
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return runir::kr::dl::semantics::detail::object_pair_indices(storage_bits(), get_num_objects()); }

    // Its return type is spelled out, so that taking the address of `indices` in another category does not deduce it.
    [[nodiscard]] auto indices(urd::index<tyr::formalism::Object> source) const
        -> decltype(runir::kr::dl::semantics::detail::object_indices(std::declval<urd::bitset_span<const ygg::uint_t>>()))
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return runir::kr::dl::semantics::detail::object_indices(value(source)); }

    // External interface: object views bound to the formalism repository, borrowing the bits and the repository.

    auto views() const
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return urd::resolve(indices(), self().context().get_formalism_repository()); }

    /// Objects reached from this source.
    auto views_from(tyr::formalism::planning::ObjectView source) const
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return urd::resolve(indices(source.index()), self().context().get_formalism_repository()); }

    static constexpr auto functions() noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
        {
            using Self = const view_mixin&;
            return urd::Functions<
                urd::function<"indices", static_cast<decltype(std::declval<Self>().indices()) (view_mixin::*)() const>(&view_mixin::indices)>,
                urd::function<"views", &view_mixin::views>,
                urd::function<"value", static_cast<decltype(std::declval<Self>().value()) (view_mixin::*)() const>(&view_mixin::value)>,
                urd::function<"storage_bits", &view_mixin::storage_bits>> {};
        }
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
        {
            using Self = const view_mixin&;
            return urd::Functions<
                urd::function<"indices", static_cast<decltype(std::declval<Self>().indices()) (view_mixin::*)() const>(&view_mixin::indices)>,
                urd::function<"indices",
                              static_cast<decltype(std::declval<Self>().indices(std::declval<urd::index<tyr::formalism::Object>>())) (view_mixin::*)(
                                  urd::index<tyr::formalism::Object>) const>(&view_mixin::indices),
                              urd::arg<"source">>,
                urd::function<"views", &view_mixin::views>,
                urd::function<"views_from", &view_mixin::views_from, urd::arg<"source">>,
                urd::function<"value",
                              static_cast<decltype(std::declval<Self>().value(std::declval<urd::index<tyr::formalism::Object>>())) (view_mixin::*)(
                                  urd::index<tyr::formalism::Object>) const>(&view_mixin::value),
                              urd::arg<"object">>,
                urd::function<"storage_bits", &view_mixin::storage_bits>> {};
        }
        else
            return urd::Functions<> {};
    }
};
}  // namespace urd

namespace runir::kr::dl::semantics
{

/// The interned and borrowed representations share the denotation interface for this category.
template<typename V, typename Category>
concept DenotationViewConcept =
    CategoryTag<Category>
    && (std::same_as<std::remove_cvref_t<V>, DenotationView<Category>> || std::same_as<std::remove_cvref_t<V>, DenotationBuilderView<Category>>);

}  // namespace runir::kr::dl::semantics

#endif
