#ifndef RUNIR_SEMANTICS_DENOTATION_BUILDER_HPP_
#define RUNIR_SEMANTICS_DENOTATION_BUILDER_HPP_

#include <yggdrasil/core/config.hpp>
#include "runir/kr/dl/semantics/denotation_index.hpp"

#include <cassert>
#include <cstddef>
#include <yggdrasil/containers/dynamic_bitset.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <tyr/formalism/object_index.hpp>
#include <utility>
#include <vector>

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

    auto get_data() noexcept -> bool& { return value; }
    auto get_data() const noexcept -> const bool& { return value; }
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

    auto get_data() noexcept -> ygg::uint_t& { return value; }
    auto get_data() const noexcept -> const ygg::uint_t& { return value; }
};

template<>
struct Builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>>
{
    using Block = ygg::uint_t;
    using Blocks = std::vector<Block>;
    using Bitset = ygg::BitsetSpan<Block>;
    using ConstBitset = ygg::BitsetSpan<const Block>;

    Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::ConceptTag>> index;
    ygg::uint_t num_objects = 0;
    Blocks blocks;

    Builder() = default;
    explicit Builder(ygg::uint_t num_objects_) { initialize(num_objects_); }
    Builder(ygg::uint_t num_objects_, Blocks blocks_) noexcept : num_objects(num_objects_), blocks(std::move(blocks_))
    {
        assert_valid_num_blocks(blocks.size(), num_objects);
        assert(get_bitset().trailing_bits_zero());
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
    {
        assert(valid_num_blocks(num_blocks_, num_objects_));
    }

    auto get_bitset() noexcept -> Bitset { return Bitset(blocks.data(), num_bits(num_objects)); }
    auto get_bitset() const noexcept -> ConstBitset { return ConstBitset(blocks.data(), num_bits(num_objects)); }
};

template<>
struct Builder<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>>
{
    using Block = ygg::uint_t;
    using Blocks = std::vector<Block>;
    using Bitset = ygg::BitsetSpan<Block>;
    using ConstBitset = ygg::BitsetSpan<const Block>;

    Index<runir::kr::dl::semantics::Denotation<runir::kr::dl::RoleTag>> index;
    ygg::uint_t num_objects = 0;
    Blocks blocks;

    Builder() = default;
    explicit Builder(ygg::uint_t num_objects_) { initialize(num_objects_); }
    Builder(ygg::uint_t num_objects_, Blocks blocks_) noexcept : num_objects(num_objects_), blocks(std::move(blocks_))
    {
        assert_valid_num_blocks(blocks.size(), num_objects);
        for (ygg::uint_t object = 0; object < num_objects; ++object)
            assert(get_row_bitset(ygg::Index<tyr::formalism::Object>(object)).trailing_bits_zero());
    }

    void initialize(ygg::uint_t num_objects_)
    {
        ygg::clear(index);
        num_objects = num_objects_;
        blocks.assign(num_blocks(num_objects), Block { 0 });
    }

    static constexpr auto num_bits(ygg::uint_t num_objects_) noexcept -> size_t
    {
        return static_cast<size_t>(num_objects_) * static_cast<size_t>(num_objects_);
    }
    static constexpr auto num_row_blocks(ygg::uint_t num_objects_) noexcept -> size_t { return Bitset::num_blocks(num_objects_); }
    static constexpr auto num_blocks(ygg::uint_t num_objects_) noexcept -> size_t { return static_cast<size_t>(num_objects_) * num_row_blocks(num_objects_); }
    static constexpr auto valid_num_blocks(size_t num_blocks_, ygg::uint_t num_objects_) noexcept -> bool { return num_blocks_ == num_blocks(num_objects_); }
    static constexpr void assert_valid_num_blocks([[maybe_unused]] size_t num_blocks_, [[maybe_unused]] ygg::uint_t num_objects_) noexcept
    {
        assert(valid_num_blocks(num_blocks_, num_objects_));
    }
    static constexpr auto row_block_offset(ygg::Index<tyr::formalism::Object> object, ygg::uint_t num_objects_) noexcept -> size_t
    {
        return static_cast<size_t>(ygg::uint_t(object)) * num_row_blocks(num_objects_);
    }

    auto get_row_bitset(ygg::Index<tyr::formalism::Object> object) noexcept -> Bitset
    {
        assert(ygg::uint_t(object) < num_objects);
        return Bitset(blocks.data() + row_block_offset(object, num_objects), num_objects);
    }

    auto get_row_bitset(ygg::Index<tyr::formalism::Object> object) const noexcept -> ConstBitset
    {
        assert(ygg::uint_t(object) < num_objects);
        return ConstBitset(blocks.data() + row_block_offset(object, num_objects), num_objects);
    }
};

}

#endif
