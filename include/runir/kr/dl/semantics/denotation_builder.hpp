#ifndef RUNIR_SEMANTICS_DENOTATION_BUILDER_HPP_
#define RUNIR_SEMANTICS_DENOTATION_BUILDER_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_records_builder.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <tuple>
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

    builder() = default;

    explicit builder(bool value_) noexcept : value(value_) {}

    void initialize(bool value_) noexcept
    {
        index = {};
        value = value_;
    }

    auto get() noexcept -> bool& { return value; }

    [[nodiscard]] auto get() const noexcept -> const bool& { return value; }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"value", &builder::value>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::value> {}; }
};

template<typename C>
struct builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>, C>
{
    urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>> index;
    urd::built_t<ygg::uint_t, C> value = 0;

    builder() = default;

    explicit builder(ygg::uint_t value_) noexcept : value(value_) {}

    void initialize(ygg::uint_t value_) noexcept
    {
        index = {};
        value = value_;
    }

    auto get() noexcept -> ygg::uint_t& { return value; }

    [[nodiscard]] auto get() const noexcept -> const ygg::uint_t& { return value; }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"value", &builder::value>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::value> {}; }
};

}  // namespace urd

/// A denotation's words, read through any of its views.
template<typename V>
struct urd::view_mixin<runir::kr::dl::semantics::DenotationBlocks, V>
{
    [[nodiscard]] decltype(auto) get_words() const { return static_cast<const V&>(*this).template member<&urd::record_of<V>::words>(); }
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
        return { held->held().words.data(), held->held().words.size() };
    if (auto* built = std::get_if<urd::built_child<DenotationBlocks, C>>(&blocks))
        return { built->held().words.data(), built->held().words.size() };
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

/// Its words made its own, `count` blocks (unspecified values): a builder of its own (its words a std::vector), the one
/// it holds kept (its buffer reused), else one drawn from `storage` (checkout<DenotationBlocks>()), read through
/// `repository`.
template<typename C, typename Storage>
std::span<ygg::uint_t> own_words(BlocksChild<C>& blocks, std::size_t count, Storage& storage, const C& repository)
{
    if (!std::holds_alternative<urd::built_child<DenotationBlocks, C>>(blocks))
        blocks.template emplace<urd::built_child<DenotationBlocks, C>>(storage.template checkout<DenotationBlocks>(), &repository);
    auto& stored = std::get<urd::built_child<DenotationBlocks, C>>(blocks).held().words;
    stored.resize(count);
    return { stored.data(), stored.size() };
}

/// Its words made its own, `count` zero blocks.
template<typename C, typename Storage>
std::span<ygg::uint_t> own_zero_words(BlocksChild<C>& blocks, std::size_t count, Storage& storage, const C& repository)
{
    const auto words = own_words(blocks, count, storage, repository);
    std::ranges::fill(words, ygg::uint_t { 0 });
    return words;
}

/// Its words made its own, a copy of `source` (a no-op where they are `source`).
template<typename C, typename Storage>
void own_copied_words(BlocksChild<C>& blocks, std::span<const ygg::uint_t> source, Storage& storage, const C& repository)
{
    if (std::holds_alternative<urd::built_child<DenotationBlocks, C>>(blocks) && words(blocks).data() == source.data())
        return;
    std::ranges::copy(source, own_words(blocks, source.size(), storage, repository).begin());
}
}  // namespace runir::kr::dl::semantics::detail

namespace urd
{

/// A concept's denotation being computed: its bitset's words a child (urd::child: its own record while it is computed,
/// interned once it is).
template<typename C>
struct builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>, C>
{
private:
    using Block = ygg::uint_t;
    using ConstBitset = urd::bitset_span<const Block>;

public:
    using Bitset = urd::bitset_span<Block>;

    urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>> index;
    urd::built_t<ygg::uint_t, C> num_objects = 0;
    urd::built_t<urd::index<runir::kr::dl::semantics::DenotationBlocks>, C> blocks;

    /// Empty over num_objects objects: its words its own, from `storage`, read through `repository`.
    template<typename Storage>
    void initialize(ygg::uint_t num_objects_, Storage& storage, const C& repository)
    {
        index = {};
        num_objects = num_objects_;
        runir::kr::dl::semantics::detail::own_zero_words(blocks, num_blocks(num_objects), storage, repository);
    }

    static constexpr auto num_bits(ygg::uint_t num_objects_) noexcept -> size_t { return num_objects_; }

    static constexpr auto num_blocks(ygg::uint_t num_objects_) noexcept -> size_t { return Bitset::num_blocks(num_bits(num_objects_)); }

    static constexpr auto valid_num_blocks(size_t num_blocks_, ygg::uint_t num_objects_) noexcept -> bool { return num_blocks_ == num_blocks(num_objects_); }

    static constexpr void assert_valid_num_blocks([[maybe_unused]] size_t num_blocks_, [[maybe_unused]] ygg::uint_t num_objects_) noexcept
    { assert(valid_num_blocks(num_blocks_, num_objects_)); }

    auto get() -> Bitset { return Bitset(runir::kr::dl::semantics::detail::own_words(blocks).data(), num_bits(num_objects)); }

    [[nodiscard]] auto get() const -> ConstBitset { return ConstBitset(runir::kr::dl::semantics::detail::words(blocks).data(), num_bits(num_objects)); }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"num_objects", &builder::num_objects>, urd::field<"blocks", &builder::blocks>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::num_objects, &builder::blocks> {}; }

    [[nodiscard]] bool contains(urd::index<tyr::formalism::Object> object) const { return get().test(static_cast<ygg::uint_t>(object.value)); }

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

    /// Includes trailing padding; bulk operations must preserve its zero bits.
    auto storage_bits() -> Bitset
    {
        const auto words = runir::kr::dl::semantics::detail::own_words(blocks);
        return Bitset(words.data(), words.size() * Bitset::Digits);
    }

    [[nodiscard]] auto storage_bits() const -> ConstBitset
    {
        const auto words = runir::kr::dl::semantics::detail::words(blocks);
        return ConstBitset(words.data(), words.size() * ConstBitset::Digits);
    }

    [[nodiscard]] bool any() const { return get().any(); }

    [[nodiscard]] auto count() const -> size_t { return get().count(); }
};

/// A role's denotation being computed: as a concept's, its rows of blocks one child.
template<typename C>
struct builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>, C>
{
private:
    using Block = ygg::uint_t;
    using ConstBitset = urd::bitset_span<const Block>;

public:
    using Bitset = urd::bitset_span<Block>;

    urd::index<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>> index;
    urd::built_t<ygg::uint_t, C> num_objects = 0;
    urd::built_t<urd::index<runir::kr::dl::semantics::DenotationBlocks>, C> blocks;

    /// Empty over num_objects objects: its words its own, from `storage`, read through `repository`.
    template<typename Storage>
    void initialize(ygg::uint_t num_objects_, Storage& storage, const C& repository)
    {
        index = {};
        num_objects = num_objects_;
        runir::kr::dl::semantics::detail::own_zero_words(blocks, num_blocks(num_objects), storage, repository);
    }

    static constexpr auto num_bits(ygg::uint_t num_objects_) noexcept -> size_t
    { return static_cast<size_t>(num_objects_) * static_cast<size_t>(num_objects_); }

    static constexpr auto num_row_blocks(ygg::uint_t num_objects_) noexcept -> size_t { return Bitset::num_blocks(num_objects_); }

    static constexpr auto num_blocks(ygg::uint_t num_objects_) noexcept -> size_t { return static_cast<size_t>(num_objects_) * num_row_blocks(num_objects_); }

    static constexpr auto valid_num_blocks(size_t num_blocks_, ygg::uint_t num_objects_) noexcept -> bool { return num_blocks_ == num_blocks(num_objects_); }

    static constexpr void assert_valid_num_blocks([[maybe_unused]] size_t num_blocks_, [[maybe_unused]] ygg::uint_t num_objects_) noexcept
    { assert(valid_num_blocks(num_blocks_, num_objects_)); }

    static constexpr auto row_block_offset(urd::index<tyr::formalism::Object> object, ygg::uint_t num_objects_) noexcept -> size_t
    { return static_cast<size_t>(static_cast<ygg::uint_t>(object.value)) * num_row_blocks(num_objects_); }

    auto get(urd::index<tyr::formalism::Object> object) -> Bitset
    {
        assert(static_cast<ygg::uint_t>(object.value) < num_objects);
        return Bitset(runir::kr::dl::semantics::detail::own_words(blocks).subspan(row_block_offset(object, num_objects)).data(), num_objects);
    }

    [[nodiscard]] auto get(urd::index<tyr::formalism::Object> object) const -> ConstBitset
    {
        assert(static_cast<ygg::uint_t>(object.value) < num_objects);
        return ConstBitset(runir::kr::dl::semantics::detail::words(blocks).subspan(row_block_offset(object, num_objects)).data(), num_objects);
    }

    [[nodiscard]] auto get_num_objects() const noexcept { return num_objects; }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"num_objects", &builder::num_objects>, urd::field<"blocks", &builder::blocks>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::num_objects, &builder::blocks> {}; }

    [[nodiscard]] bool contains(urd::index<tyr::formalism::Object> source, urd::index<tyr::formalism::Object> target) const
    { return get(source).test(static_cast<ygg::uint_t>(target.value)); }

    /// Set membership and invalidate published identity; return whether it changed.
    bool set(urd::index<tyr::formalism::Object> source, urd::index<tyr::formalism::Object> target, bool present)
    {
        auto row = get(source);
        const auto changed = row.test(static_cast<ygg::uint_t>(target.value)) != present;
        row.set(static_cast<ygg::uint_t>(target.value), present);
        index = {};
        return changed;
    }

    void assign_row(urd::index<tyr::formalism::Object> source, ConstBitset row)
    {
        get(source).copy_from(row);
        index = {};
    }

    void flip()
    {
        for (ygg::uint_t source = 0; source < num_objects; ++source)
            get(urd::index<tyr::formalism::Object>(source)).flip();
        index = {};
    }

    /// Includes per-row padding; bulk operations must preserve its zero bits.
    auto storage_bits() -> Bitset
    {
        const auto words = runir::kr::dl::semantics::detail::own_words(blocks);
        return Bitset(words.data(), words.size() * Bitset::Digits);
    }

    [[nodiscard]] auto storage_bits() const -> ConstBitset
    {
        const auto words = runir::kr::dl::semantics::detail::words(blocks);
        return ConstBitset(words.data(), words.size() * ConstBitset::Digits);
    }

    [[nodiscard]] bool any() const { return storage_bits().any(); }

    [[nodiscard]] auto count() const -> size_t { return storage_bits().count(); }
};

}  // namespace urd

#endif
