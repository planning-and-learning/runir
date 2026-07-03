#ifndef RUNIR_KR_DL_REGISTER_INDEX_HPP_
#define RUNIR_KR_DL_REGISTER_INDEX_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/ids/index_mixins.hpp>

namespace ygg
{

template<>
struct Index<runir::kr::dl::RegisterTag> : IndexMixin<Index<runir::kr::dl::RegisterTag>>
{
    using Base = IndexMixin<Index<runir::kr::dl::RegisterTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
