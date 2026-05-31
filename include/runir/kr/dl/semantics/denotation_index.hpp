#ifndef RUNIR_SEMANTICS_DENOTATION_INDEX_HPP_
#define RUNIR_SEMANTICS_DENOTATION_INDEX_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"

#include <concepts>
#include <limits>
#include <stdexcept>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::dl::semantics::Denotation<Category>> : IndexMixin<Index<runir::kr::dl::semantics::Denotation<Category>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::semantics::Denotation<Category>>>;
    using Base::Base;
};

}

namespace runir::kr::dl::semantics
{

template<typename Index, std::unsigned_integral Block>
struct IndexCoder
{
    using value_type = Index;

    static constexpr value_type decode(Block block) noexcept { return value_type(static_cast<ygg::uint_t>(block)); }

    static constexpr Block encode(value_type value)
    {
        const auto raw = ygg::uint_t(value);

        if (raw > std::numeric_limits<Block>::max())
            throw std::out_of_range("IndexCoder: index does not fit into block");

        return static_cast<Block>(raw);
    }
};

}

#endif
