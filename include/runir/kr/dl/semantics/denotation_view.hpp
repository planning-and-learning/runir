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
#include <tyr/formalism/repository.hpp>
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
    using Type = tyr::formalism::ObjectView;
};

template<>
struct DenotationElementType<runir::kr::dl::RoleTag>
{
    using Type = std::pair<tyr::formalism::ObjectView, tyr::formalism::ObjectView>;
};

template<runir::kr::dl::CategoryTag Category>
using DenotationElementView = typename DenotationElementType<Category>::Type;

template<runir::kr::dl::CategoryTag Category>
using DenotationElementViewList = urd::Store<urd::sequence<DenotationElementView<Category>>, urd::contiguous>;

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

    /// What it denotes, its words read in place (a stored record's, or a builder's), iterated as the views of its
    /// objects (a concept's) or of its pairs of objects (a role's).
    auto get_values() const
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return self().template member<&urd::record_of<V>::values>(); }

    // Internal interface: raw values, bitsets and typed indices avoid constructing object views.

    auto value() const
        requires(std::same_as<Category, runir::kr::dl::ConceptTag>)
    {
        using Layout = runir::kr::dl::semantics::detail::ConceptLayout;
        using Bitset = urd::bitset_span<const ygg::uint_t>;

        const auto bits = get_values().bits();
        return Bitset(bits.words().data(), Layout::num_bits(bits.num_objects()));
    }

    auto value(urd::index<::tyr::formalism::Object> object) const
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        using Layout = runir::kr::dl::semantics::detail::RoleLayout;
        using Bitset = urd::bitset_span<const ygg::uint_t>;

        const auto bits = get_values().bits();
        assert(ygg::uint_t(object.value) < bits.num_objects());
        return Bitset(bits.words().data() + Layout::row_block_offset(object, bits.num_objects()), bits.num_objects());
    }

    /// Whether `object` is in a concept's denotation.
    [[nodiscard]] bool contains(urd::index<::tyr::formalism::Object> object) const
        requires(std::same_as<Category, runir::kr::dl::ConceptTag>)
    { return value().test(static_cast<ygg::uint_t>(object.value)); }

    /// Whether (source, target) is in a role's denotation.
    [[nodiscard]] bool contains(urd::index<::tyr::formalism::Object> source, urd::index<::tyr::formalism::Object> target) const
        requires(std::same_as<Category, runir::kr::dl::RoleTag>)
    { return value(source).test(static_cast<ygg::uint_t>(target.value)); }

    /// Includes zero padding: trailing concept bits or padding after each role row.
    auto storage_bits() const
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return get_values().bits().storage_bits(); }

    /// Typed indices borrowing only the bits, independently of the view and range wrappers.
    // One function, not an overload per category: functions() takes its address, which deduces the return type of every
    // overload, and AppleClang does not mangle requires-clauses, so a concept and a role overload would share a symbol.
    auto indices() const
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return get_values().bits().elements(); }

    /// The targets of `source` (a role's row): only a role denotation has rows.
    [[nodiscard]] runir::kr::dl::semantics::detail::ObjectIndices indices(urd::index<tyr::formalism::Object> source) const
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return runir::kr::dl::semantics::detail::object_indices(value(source)); }

    // External interface: object views bound to the formalism repository, borrowing the bits and the repository.

    auto views() const
        requires runir::kr::dl::ConceptOrRoleTag<Category>
    { return urd::resolve(indices(), self().context().template get<tyr::formalism::Repository>()); }

    /// Objects reached from this source.
    auto views_from(tyr::formalism::ObjectView source) const
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return urd::resolve(indices(source.index()), self().context().template get<tyr::formalism::Repository>()); }

    static constexpr auto functions() noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
        {
            using Self = const view_mixin&;
            return urd::Functions<urd::function<"indices", static_cast<decltype(std::declval<Self>().indices()) (view_mixin::*)() const>(&view_mixin::indices)>,
                                  urd::function<"views", &view_mixin::views>,
                                  urd::function<"value", static_cast<decltype(std::declval<Self>().value()) (view_mixin::*)() const>(&view_mixin::value)>,
                                  urd::function<"contains",
                                                static_cast<bool (view_mixin::*)(urd::index<tyr::formalism::Object>) const>(&view_mixin::contains),
                                                urd::arg<"object">>,
                                  urd::function<"storage_bits", &view_mixin::storage_bits>> {};
        }
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
        {
            using Self = const view_mixin&;
            return urd::Functions<urd::function<"indices", static_cast<decltype(std::declval<Self>().indices()) (view_mixin::*)() const>(&view_mixin::indices)>,
                                  urd::function<"indices",
                                                static_cast<decltype(std::declval<Self>().indices(std::declval<urd::index<tyr::formalism::Object>>())) (
                                                    view_mixin::*)(urd::index<tyr::formalism::Object>) const>(&view_mixin::indices),
                                                urd::arg<"source">>,
                                  urd::function<"views", &view_mixin::views>,
                                  urd::function<"views_from", &view_mixin::views_from, urd::arg<"source">>,
                                  urd::function<"value",
                                                static_cast<decltype(std::declval<Self>().value(std::declval<urd::index<tyr::formalism::Object>>())) (
                                                    view_mixin::*)(urd::index<tyr::formalism::Object>) const>(&view_mixin::value),
                                                urd::arg<"object">>,
                                  urd::function<"contains",
                                                static_cast<bool (view_mixin::*)(urd::index<tyr::formalism::Object>, urd::index<tyr::formalism::Object>) const>(
                                                    &view_mixin::contains),
                                                urd::arg<"source">,
                                                urd::arg<"target">>,
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
