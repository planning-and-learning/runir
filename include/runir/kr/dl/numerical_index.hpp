#ifndef RUNIR_NUMERICAL_INDEX_HPP_
#define RUNIR_NUMERICAL_INDEX_HPP_

#include "runir/kr/dl/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::NumericalConstructorTag Tag>
struct Index<runir::kr::dl::Numerical<Tag>> : IndexMixin<Index<runir::kr::dl::Numerical<Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::Numerical<Tag>>>;
    using Base::Base;
};

}

#endif
