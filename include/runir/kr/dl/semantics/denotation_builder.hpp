#ifndef RUNIR_SEMANTICS_DENOTATION_BUILDER_HPP_
#define RUNIR_SEMANTICS_DENOTATION_BUILDER_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_records_builder.hpp"

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
    urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>> index;
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
    urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>> index;
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

/// A denotation's words, read through any of its views.
template<typename V>
struct urd::view_mixin<runir::kr::dl::semantics::DenotationBlocks, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] decltype(auto) get_words() const { return self().template member<&urd::record_of<V>::words>(); }
};

namespace runir::kr::dl::semantics::detail
{
template<typename C>
using BlocksChild = urd::child<DenotationBlocks, C>;

/// A denotation builder's words to write: its own (a record of its own, or a builder). Interned words are shared and are
/// not written: initialize and assign give a builder words of its own (std::logic_error otherwise).
template<typename C>
std::span<ygg::uint_t> own_words(BlocksChild<C>& blocks)
{
    if (auto* held = std::get_if<urd::held_child<DenotationBlocks, C>>(&blocks))
        return { (*held)->words.data(), (*held)->words.size() };
    if (auto* built = std::get_if<urd::built_child<DenotationBlocks, C>>(&blocks))
        return { (*built)->words.data(), (*built)->words.size() };
    throw std::logic_error("Denotation: its words are interned (shared); initialize or assign gives it words of its own");
}

/// A denotation's words to read, from its blocks in whichever form they are held or resolved (a view, or a variant of
/// views: a child, or a builder view's resolved member).
template<typename Blocks>
std::span<const ygg::uint_t> words(const Blocks& blocks)
{
    return urd::visit_view(blocks,
                           [](const auto& view) -> std::span<const ygg::uint_t>
                           {
                               const auto& stored = view.get_words();
                               return { stored.data(), stored.size() };
                           });
}

/// A denotation builder's words to read, as its child holds them: its own (a record of its own, or a builder). Interned
/// words are read through a view of the builder, which has its context (std::logic_error here).
template<typename C>
std::span<const ygg::uint_t> words(const BlocksChild<C>& blocks)
{
    if (const auto* held = std::get_if<urd::held_child<DenotationBlocks, C>>(&blocks))
        return { (*held)->words.data(), (*held)->words.size() };
    if (const auto* built = std::get_if<urd::built_child<DenotationBlocks, C>>(&blocks))
        return { (*built)->words.data(), (*built)->words.size() };
    throw std::logic_error("Denotation: its words are interned: read them through a view of it, which has its repository");
}

/// Its words made its own, `count` blocks (unspecified values): a builder of its own (its words a std::vector), the one
/// it holds kept (its buffer reused), else one drawn from `storage` (checkout<DenotationBlocks>()).
template<typename C, typename Storage>
std::span<ygg::uint_t> own_words(BlocksChild<C>& blocks, std::size_t count, Storage& storage)
{
    if (!std::holds_alternative<urd::built_child<DenotationBlocks, C>>(blocks))
        blocks.template emplace<urd::built_child<DenotationBlocks, C>>(storage.template checkout<DenotationBlocks>());
    auto& stored = std::get<urd::built_child<DenotationBlocks, C>>(blocks)->words;
    stored.resize(count);
    return { stored.data(), stored.size() };
}

/// Its words made its own, `count` zero blocks.
template<typename C, typename Storage>
std::span<ygg::uint_t> own_zero_words(BlocksChild<C>& blocks, std::size_t count, Storage& storage)
{
    const auto words = own_words(blocks, count, storage);
    std::ranges::fill(words, ygg::uint_t { 0 });
    return words;
}

/// Its words made its own, a copy of `source` (a no-op where they are `source`).
template<typename C, typename Storage>
void own_copied_words(BlocksChild<C>& blocks, std::span<const ygg::uint_t> source, Storage& storage)
{
    if (std::holds_alternative<urd::built_child<DenotationBlocks, C>>(blocks) && words(blocks).data() == source.data())
        return;
    std::ranges::copy(source, own_words(blocks, source.size(), storage).begin());
}
/// Where a concept's bits lie in its words: a bit per object, the trailing bits zero.
struct ConceptLayout
{
    static constexpr auto num_bits(ygg::uint_t num_objects) noexcept -> size_t { return num_objects; }

    static constexpr auto num_blocks(ygg::uint_t num_objects) noexcept -> size_t { return urd::bitset_span<ygg::uint_t>::num_blocks(num_bits(num_objects)); }
};

/// Where a role's bits lie in its words: a row of whole blocks per source object, each row's trailing bits zero.
struct RoleLayout
{
    static constexpr auto num_bits(ygg::uint_t num_objects) noexcept -> size_t
    { return static_cast<size_t>(num_objects) * static_cast<size_t>(num_objects); }

    static constexpr auto num_row_blocks(ygg::uint_t num_objects) noexcept -> size_t { return urd::bitset_span<ygg::uint_t>::num_blocks(num_objects); }

    static constexpr auto num_blocks(ygg::uint_t num_objects) noexcept -> size_t { return static_cast<size_t>(num_objects) * num_row_blocks(num_objects); }

    static constexpr auto row_block_offset(urd::index<tyr::formalism::Object> object, ygg::uint_t num_objects) noexcept -> size_t
    { return static_cast<size_t>(static_cast<ygg::uint_t>(object.value)) * num_row_blocks(num_objects); }
};
}  // namespace runir::kr::dl::semantics::detail

namespace urd
{

/// A concept's denotation being computed: its bitset's words a child (urd::child: its own record while it is computed,
/// interned once it is).
template<typename C>
struct builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>, C>
{
    urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>> index;
    urd::built_t<ygg::uint_t, C> num_objects = 0;
    urd::built_t<urd::index<runir::kr::dl::semantics::DenotationBlocks>, C> blocks;

    /// Its bits, its words made its own (a read is its view's: value()).
    auto get() -> urd::bitset_span<ygg::uint_t>
    {
        return urd::bitset_span<ygg::uint_t>(runir::kr::dl::semantics::detail::own_words(blocks).data(),
                                             runir::kr::dl::semantics::detail::ConceptLayout::num_bits(num_objects));
    }

    /// Set membership and invalidate published identity; return whether it changed.
    bool set(urd::index<tyr::formalism::Object> object, bool present)
    {
        auto bits = get();
        const auto changed = bits.test(static_cast<ygg::uint_t>(object.value)) != present;
        bits.set(static_cast<ygg::uint_t>(object.value), present);
        index = {};
        return changed;
    }

    void flip()
    {
        get().flip();
        index = {};
    }

    /// Its words as bits, trailing padding included; bulk operations must preserve its zero bits.
    auto storage_bits() -> urd::bitset_span<ygg::uint_t>
    {
        const auto words = runir::kr::dl::semantics::detail::own_words(blocks);
        return urd::bitset_span<ygg::uint_t>(words.data(), words.size() * urd::bitset_span<ygg::uint_t>::Digits);
    }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"num_objects", &builder::num_objects>, urd::field<"blocks", &builder::blocks>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::num_objects, &builder::blocks> {}; }

    static constexpr auto functions() noexcept
    {
        return urd::Functions<urd::function<"get", &builder::get>,
                              urd::function<"set", &builder::set, urd::arg<"object">, urd::arg<"present">>,
                              urd::function<"flip", &builder::flip>,
                              urd::function<"storage_bits", &builder::storage_bits>> {};
    }
};

/// A role's denotation being computed: as a concept's, its rows of blocks one child.
template<typename C>
struct builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>, C>
{
    urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>> index;
    urd::built_t<ygg::uint_t, C> num_objects = 0;
    urd::built_t<urd::index<runir::kr::dl::semantics::DenotationBlocks>, C> blocks;

    /// The row of `object`, its words made its own (a read is its view's: value(object)).
    auto get(urd::index<tyr::formalism::Object> object) -> urd::bitset_span<ygg::uint_t>
    {
        assert(static_cast<ygg::uint_t>(object.value) < num_objects);
        return urd::bitset_span<ygg::uint_t>(
            runir::kr::dl::semantics::detail::own_words(blocks)
                .subspan(runir::kr::dl::semantics::detail::RoleLayout::row_block_offset(object, num_objects))
                .data(),
            num_objects);
    }

    /// Set membership and invalidate published identity; return whether it changed.
    bool set(urd::index<tyr::formalism::Object> source, urd::index<tyr::formalism::Object> target, bool present)
    {
        auto row = get(source);
        const auto changed = row.test(static_cast<ygg::uint_t>(target.value)) != present;
        row.set(static_cast<ygg::uint_t>(target.value), present);
        index = {};
        return changed;
    }

    void flip()
    {
        for (ygg::uint_t source = 0; source < num_objects; ++source)
            get(urd::index<tyr::formalism::Object>(source)).flip();
        index = {};
    }

    /// Its words as bits, per-row padding included; bulk operations must preserve its zero bits.
    auto storage_bits() -> urd::bitset_span<ygg::uint_t>
    {
        const auto words = runir::kr::dl::semantics::detail::own_words(blocks);
        return urd::bitset_span<ygg::uint_t>(words.data(), words.size() * urd::bitset_span<ygg::uint_t>::Digits);
    }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"num_objects", &builder::num_objects>, urd::field<"blocks", &builder::blocks>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::num_objects, &builder::blocks> {}; }

    static constexpr auto functions() noexcept
    {
        return urd::Functions<urd::function<"get", &builder::get, urd::arg<"object">>,
                              urd::function<"set", &builder::set, urd::arg<"source">, urd::arg<"target">, urd::arg<"present">>,
                              urd::function<"flip", &builder::flip>,
                              urd::function<"storage_bits", &builder::storage_bits>> {};
    }
};

}  // namespace urd

namespace runir::kr::dl::semantics
{

/// A concept's or role's denotation builder emptied over num_objects objects: its words its own, drawn from `storage`.
template<ConceptOrRoleTag Category, typename C, typename Storage>
void initialize(urd::builder<Denotation<Category>, C>& built, ygg::uint_t num_objects, Storage& storage)
{
    using Layout = std::conditional_t<std::same_as<Category, ConceptTag>, detail::ConceptLayout, detail::RoleLayout>;
    built.index = {};
    built.num_objects = num_objects;
    detail::own_zero_words(built.blocks, Layout::num_blocks(num_objects), storage);
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
