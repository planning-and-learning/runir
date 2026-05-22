#ifndef RUNIR_KR_MPG_DL_CONSTRUCTOR_INDEX_HPP_
#define RUNIR_KR_MPG_DL_CONSTRUCTOR_INDEX_HPP_

#include "runir/kr/mpg/dl/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::mpg::dl::Constructor<Category>> : IndexMixin<Index<runir::kr::mpg::dl::Constructor<Category>>>
{
    using Base = IndexMixin<Index<runir::kr::mpg::dl::Constructor<Category>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
