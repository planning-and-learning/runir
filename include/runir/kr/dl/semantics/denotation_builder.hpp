#ifndef RUNIR_SEMANTICS_DENOTATION_BUILDER_HPP_
#define RUNIR_SEMANTICS_DENOTATION_BUILDER_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"

#include <cassert>
#include <cstddef>
#include <span>
#include <tuple>
#include <tyr/formalism/declarations.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/containers/dynamic_bitset.hpp>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>>
{
    Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::BooleanTag>> index;
    bool value = false;

    Builder() = default;
    explicit Builder(bool value_) noexcept : value(value_) {}

    void initialize(bool value_) noexcept
    {
        ygg::clear(index);
        value = value_;
    }

    auto get() noexcept -> bool& { return value; }
    [[nodiscard]] auto get() const noexcept -> const bool& { return value; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Builder::value> {}; }
};

template<>
struct Builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>>
{
    Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::NumericalTag>> index;
    ygg::uint_t value = 0;

    Builder() = default;
    explicit Builder(ygg::uint_t value_) noexcept : value(value_) {}

    void initialize(ygg::uint_t value_) noexcept
    {
        ygg::clear(index);
        value = value_;
    }

    auto get() noexcept -> ygg::uint_t& { return value; }
    [[nodiscard]] auto get() const noexcept -> const ygg::uint_t& { return value; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Builder::value> {}; }
};

template<>
struct Builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>
{
private:
    using Block = ygg::uint_t;
    using Blocks = std::vector<Block>;
    using ConstBitset = ygg::BitsetSpan<const Block>;

public:
    using Bitset = ygg::BitsetSpan<Block>;

    Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>> index;
    ygg::uint_t num_objects = 0;
    Blocks blocks;

    Builder() = default;
    explicit Builder(ygg::uint_t num_objects_) { initialize(num_objects_); }
    Builder(ygg::uint_t num_objects_, Blocks blocks_) noexcept : num_objects(num_objects_), blocks(std::move(blocks_))
    {
        assert_valid_num_blocks(blocks.size(), num_objects);
        assert(get().trailing_bits_zero());
    }

    void initialize(ygg::uint_t num_objects_)
    {
        ygg::clear(index);
        num_objects = num_objects_;
        blocks.assign(num_blocks(num_objects), Block { 0 });
    }

    static constexpr auto num_bits(ygg::uint_t num_objects_) noexcept -> size_t { return num_objects_; }
    static constexpr auto num_blocks(ygg::uint_t num_objects_) noexcept -> size_t { return Bitset::num_blocks(num_bits(num_objects_)); }
    static constexpr auto valid_num_blocks(size_t num_blocks_, ygg::uint_t num_objects_) noexcept -> bool { return num_blocks_ == num_blocks(num_objects_); }
    static constexpr void assert_valid_num_blocks([[maybe_unused]] size_t num_blocks_, [[maybe_unused]] ygg::uint_t num_objects_) noexcept
    { assert(valid_num_blocks(num_blocks_, num_objects_)); }

    auto get() noexcept -> Bitset { return Bitset(blocks.data(), num_bits(num_objects)); }
    [[nodiscard]] auto get() const noexcept -> ConstBitset { return ConstBitset(blocks.data(), num_bits(num_objects)); }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Builder::num_objects, &Builder::blocks> {}; }

    [[nodiscard]] bool contains(Index<tyr::formalism::Object> object) const noexcept { return get().test(static_cast<ygg::uint_t>(object)); }

    /// Set membership and invalidate published identity; return whether it changed.
    bool set(Index<tyr::formalism::Object> object, bool present) noexcept
    {
        auto bits = get();
        const auto changed = bits.test(static_cast<ygg::uint_t>(object)) != present;
        bits.set(static_cast<ygg::uint_t>(object), present);
        ygg::clear(index);
        return changed;
    }

    void flip() noexcept
    {
        get().flip();
        ygg::clear(index);
    }

    /// Includes trailing padding; bulk operations must preserve its zero bits.
    auto storage_bits() noexcept -> Bitset { return Bitset(blocks.data(), blocks.size() * Bitset::Digits); }
    [[nodiscard]] auto storage_bits() const noexcept -> ConstBitset { return ConstBitset(blocks.data(), blocks.size() * ConstBitset::Digits); }

    [[nodiscard]] bool any() const noexcept { return get().any(); }
    [[nodiscard]] auto count() const noexcept -> size_t { return get().count(); }
};

template<>
struct Builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>
{
private:
    using Block = ygg::uint_t;
    using Blocks = std::vector<Block>;
    using ConstBitset = ygg::BitsetSpan<const Block>;

public:
    using Bitset = ygg::BitsetSpan<Block>;

    Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>> index;
    ygg::uint_t num_objects = 0;
    Blocks blocks;

    Builder() = default;
    explicit Builder(ygg::uint_t num_objects_) { initialize(num_objects_); }
    Builder(ygg::uint_t num_objects_, Blocks blocks_) noexcept : num_objects(num_objects_), blocks(std::move(blocks_))
    {
        assert_valid_num_blocks(blocks.size(), num_objects);
        for (ygg::uint_t object = 0; object < num_objects; ++object)
            assert(get(object).trailing_bits_zero());
    }

    void initialize(ygg::uint_t num_objects_)
    {
        ygg::clear(index);
        num_objects = num_objects_;
        blocks.assign(num_blocks(num_objects), Block { 0 });
    }

    static constexpr auto num_bits(ygg::uint_t num_objects_) noexcept -> size_t
    { return static_cast<size_t>(num_objects_) * static_cast<size_t>(num_objects_); }
    static constexpr auto num_row_blocks(ygg::uint_t num_objects_) noexcept -> size_t { return Bitset::num_blocks(num_objects_); }
    static constexpr auto num_blocks(ygg::uint_t num_objects_) noexcept -> size_t { return static_cast<size_t>(num_objects_) * num_row_blocks(num_objects_); }
    static constexpr auto valid_num_blocks(size_t num_blocks_, ygg::uint_t num_objects_) noexcept -> bool { return num_blocks_ == num_blocks(num_objects_); }
    static constexpr void assert_valid_num_blocks([[maybe_unused]] size_t num_blocks_, [[maybe_unused]] ygg::uint_t num_objects_) noexcept
    { assert(valid_num_blocks(num_blocks_, num_objects_)); }
    static constexpr auto row_block_offset(ygg::Index<tyr::formalism::Object> object, ygg::uint_t num_objects_) noexcept -> size_t
    { return static_cast<size_t>(static_cast<ygg::uint_t>(object)) * num_row_blocks(num_objects_); }

    auto get(ygg::Index<tyr::formalism::Object> object) noexcept -> Bitset
    {
        assert(static_cast<ygg::uint_t>(object) < num_objects);
        return Bitset(std::span(blocks).subspan(row_block_offset(object, num_objects)).data(), num_objects);
    }

    [[nodiscard]] auto get(ygg::Index<tyr::formalism::Object> object) const noexcept -> ConstBitset
    {
        assert(static_cast<ygg::uint_t>(object) < num_objects);
        return ConstBitset(std::span(blocks).subspan(row_block_offset(object, num_objects)).data(), num_objects);
    }

    auto get(ygg::uint_t object) noexcept -> Bitset { return get(ygg::Index<tyr::formalism::Object>(object)); }
    [[nodiscard]] auto get(ygg::uint_t object) const noexcept -> ConstBitset { return get(ygg::Index<tyr::formalism::Object>(object)); }
    [[nodiscard]] auto get_num_objects() const noexcept { return num_objects; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Builder::num_objects, &Builder::blocks> {}; }

    [[nodiscard]] bool contains(Index<tyr::formalism::Object> source, Index<tyr::formalism::Object> target) const noexcept
    { return get(source).test(static_cast<ygg::uint_t>(target)); }

    /// Set membership and invalidate published identity; return whether it changed.
    bool set(Index<tyr::formalism::Object> source, Index<tyr::formalism::Object> target, bool present) noexcept
    {
        auto row = get(source);
        const auto changed = row.test(static_cast<ygg::uint_t>(target)) != present;
        row.set(static_cast<ygg::uint_t>(target), present);
        ygg::clear(index);
        return changed;
    }

    void assign_row(Index<tyr::formalism::Object> source, ConstBitset row)
    {
        get(source).copy_from(row);
        ygg::clear(index);
    }

    void flip() noexcept
    {
        for (ygg::uint_t source = 0; source < num_objects; ++source)
            get(source).flip();
        ygg::clear(index);
    }

    /// Includes per-row padding; bulk operations must preserve its zero bits.
    auto storage_bits() noexcept -> Bitset { return Bitset(blocks.data(), blocks.size() * Bitset::Digits); }
    [[nodiscard]] auto storage_bits() const noexcept -> ConstBitset { return ConstBitset(blocks.data(), blocks.size() * ConstBitset::Digits); }

    [[nodiscard]] bool any() const noexcept { return storage_bits().any(); }
    [[nodiscard]] auto count() const noexcept -> size_t { return storage_bits().count(); }
};

}

#endif
