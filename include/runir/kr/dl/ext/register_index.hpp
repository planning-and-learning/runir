#ifndef RUNIR_KR_DL_EXT_REGISTER_INDEX_HPP_
#define RUNIR_KR_DL_EXT_REGISTER_INDEX_HPP_

#include "runir/kr/dl/ext/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::dl::RegisterTag> : IndexMixin<Index<runir::kr::dl::RegisterTag>>
{
    using Base = IndexMixin<Index<runir::kr::dl::RegisterTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
