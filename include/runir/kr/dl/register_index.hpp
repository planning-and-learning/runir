#ifndef RUNIR_KR_DL_REGISTER_INDEX_HPP_
#define RUNIR_KR_DL_REGISTER_INDEX_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/ids/index_mixins.hpp>

namespace ygg
{

template<runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::dl::Register<Category>> : IndexMixin<Index<runir::kr::dl::Register<Category>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::Register<Category>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
