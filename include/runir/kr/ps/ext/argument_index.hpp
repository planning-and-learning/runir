#ifndef RUNIR_KR_PS_EXT_ARGUMENT_INDEX_HPP_
#define RUNIR_KR_PS_EXT_ARGUMENT_INDEX_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::ps::ext::Argument<Category>> : IndexMixin<Index<runir::kr::ps::ext::Argument<Category>>>
{
    using Base = IndexMixin<Index<runir::kr::ps::ext::Argument<Category>>>;
    using Base::Base;
};

}  // namespace tyr

#endif
