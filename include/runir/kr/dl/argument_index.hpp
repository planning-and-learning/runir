#ifndef RUNIR_KR_DL_ARGUMENT_INDEX_HPP_
#define RUNIR_KR_DL_ARGUMENT_INDEX_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/ids/index_mixins.hpp>

namespace ygg
{

template<runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::dl::Argument<Category>> : IndexMixin<Index<runir::kr::dl::Argument<Category>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::Argument<Category>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
