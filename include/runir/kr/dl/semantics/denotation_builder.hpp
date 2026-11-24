#ifndef RUNIR_SEMANTICS_DENOTATION_BUILDER_HPP_
#define RUNIR_SEMANTICS_DENOTATION_BUILDER_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_data.hpp"

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <urd/bits.hpp>
#include <urd/elements/builder.hpp>
#include <urd/elements/flat.hpp>
#include <urd/elements/view.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/core/config.hpp>

namespace urd
{

template<typename C>
struct builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>, C>
{
    urd::key<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>> index;
    urd::built_t<bool, C> value = false;

    void initialize(bool value_) noexcept
    {
        index = {};
        value = value_;
    }

    auto get() noexcept -> bool& { return value; }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"value", &builder::value>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::value> {}; }

    static constexpr auto functions() noexcept
    { return urd::Functions<urd::function<"initialize", &builder::initialize, urd::arg<"value">>, urd::function<"get", &builder::get>> {}; }
};

template<typename C>
struct builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>, C>
{
    urd::key<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>> index;
    urd::built_t<ygg::uint_t, C> value = 0;

    void initialize(ygg::uint_t value_) noexcept
    {
        index = {};
        value = value_;
    }

    auto get() noexcept -> ygg::uint_t& { return value; }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"value", &builder::value>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::value> {}; }

    static constexpr auto functions() noexcept
    { return urd::Functions<urd::function<"initialize", &builder::initialize, urd::arg<"value">>, urd::function<"get", &builder::get>> {}; }
};

}  // namespace urd

namespace urd
{

/// A concept's or role's denotation being computed: its values a bitset of its own (DenotationBitset), set in place; a
/// role's rows of blocks are one bitset, as a concept's bits are.
template<runir::kr::dl::ConceptOrRoleTag Category, typename C>
struct builder<runir::kr::dl::semantics::Denotation<Category>, C>
{
    urd::key<runir::kr::dl::semantics::Denotation<Category>> index;
    urd::built_t<ygg::uint_t, C> num_objects = 0;
    urd::built_t<runir::kr::dl::semantics::DenotationWords<Category>, C> values;

    /// A concept's bits (a read is its view's: value()).
    auto get() -> urd::bitset_span<ygg::uint_t>
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    { return urd::bitset_span<ygg::uint_t>(values.words.data(), runir::kr::dl::semantics::detail::ConceptLayout::num_bits(num_objects)); }

    /// A role's row of `object` (a read is its view's: value(object)).
    auto get(urd::index<tyr::formalism::Object> object) -> urd::bitset_span<ygg::uint_t>
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        assert(static_cast<ygg::uint_t>(object.value) < num_objects);
        const auto words = std::span<ygg::uint_t>(values.words.data(), values.words.size());
        return urd::bitset_span<ygg::uint_t>(words.subspan(runir::kr::dl::semantics::detail::RoleLayout::row_block_offset(object, num_objects)).data(),
                                             num_objects);
    }

    /// Set a concept's membership and invalidate published identity; return whether it changed.
    bool set(urd::index<tyr::formalism::Object> object, bool present)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        auto bits = get();
        const auto changed = bits.test(static_cast<ygg::uint_t>(object.value)) != present;
        bits.set(static_cast<ygg::uint_t>(object.value), present);
        index = {};
        return changed;
    }

    /// Set a role's membership and invalidate published identity; return whether it changed.
    bool set(urd::index<tyr::formalism::Object> source, urd::index<tyr::formalism::Object> target, bool present)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        auto row = get(source);
        const auto changed = row.test(static_cast<ygg::uint_t>(target.value)) != present;
        row.set(static_cast<ygg::uint_t>(target.value), present);
        index = {};
        return changed;
    }

    /// Complement it: a concept's bits, or each of a role's rows (its padding stays zero).
    void flip()
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            get().flip();
        else
            for (ygg::uint_t source = 0; source < num_objects; ++source)
                get(urd::index<tyr::formalism::Object>(source)).flip();
        index = {};
    }

    /// Its words as bits, padding included (a concept's trailing bits, a role's per row); bulk operations must preserve
    /// its zero bits.
    auto storage_bits() -> urd::bitset_span<ygg::uint_t>
    { return urd::bitset_span<ygg::uint_t>(values.words.data(), values.words.size() * urd::bitset_span<ygg::uint_t>::Digits); }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"num_objects", &builder::num_objects>, urd::field<"values", &builder::values>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto functions() noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
        {
            return urd::Functions<urd::function<"get", static_cast<urd::bitset_span<ygg::uint_t> (builder::*)()>(&builder::get)>,
                                  urd::function<"set",
                                                static_cast<bool (builder::*)(urd::index<tyr::formalism::Object>, bool)>(&builder::set),
                                                urd::arg<"object">,
                                                urd::arg<"present">>,
                                  urd::function<"flip", &builder::flip>,
                                  urd::function<"storage_bits", &builder::storage_bits>> {};
        }
        else
        {
            return urd::Functions<
                urd::function<"get",
                              static_cast<urd::bitset_span<ygg::uint_t> (builder::*)(urd::index<tyr::formalism::Object>)>(&builder::get),
                              urd::arg<"object">>,
                urd::function<"set",
                              static_cast<bool (builder::*)(urd::index<tyr::formalism::Object>, urd::index<tyr::formalism::Object>, bool)>(&builder::set),
                              urd::arg<"source">,
                              urd::arg<"target">,
                              urd::arg<"present">>,
                urd::function<"flip", &builder::flip>,
                urd::function<"storage_bits", &builder::storage_bits>> {};
        }
    }
};

}  // namespace urd

namespace runir::kr::dl::semantics
{

/// A concept's or role's denotation builder emptied over num_objects objects: zero words, its buffer kept (`storage`
/// holds no words of it).
template<ConceptOrRoleTag Category, typename C, typename Storage>
void initialize(urd::builder<Denotation<Category>, C>& built, ygg::uint_t num_objects, Storage& /*storage*/)
{
    built.index = {};
    built.num_objects = num_objects;
    built.values.initialize(num_objects);
}

/// A role's denotation builder with the row of `source` copied from `row` (another denotation's, read through its
/// view).
template<typename C>
void assign_row(urd::builder<Denotation<RoleTag>, C>& built, urd::index<tyr::formalism::Object> source, urd::bitset_span<const ygg::uint_t> row)
{
    built.get(source).copy_from(row);
    built.index = {};
}

}  // namespace runir::kr::dl::semantics

#endif
