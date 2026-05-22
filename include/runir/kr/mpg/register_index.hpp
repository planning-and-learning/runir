#ifndef RUNIR_KR_MPG_REGISTER_INDEX_HPP_
#define RUNIR_KR_MPG_REGISTER_INDEX_HPP_

#include "runir/kr/mpg/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::mpg::Register> : IndexMixin<Index<runir::kr::mpg::Register>>
{
    using Base = IndexMixin<Index<runir::kr::mpg::Register>>;
    using Base::Base;
};

}  // namespace tyr

#endif
