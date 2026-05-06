#ifndef RUNIR_NUMERICAL_INDEX_HPP_
#define RUNIR_NUMERICAL_INDEX_HPP_

#include "runir/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::NumericalConstructorTag Tag>
struct Index<runir::Numerical<Tag>> : IndexMixin<Index<runir::Numerical<Tag>>>
{
    using Base = IndexMixin<Index<runir::Numerical<Tag>>>;
    using Base::Base;
};

}

#endif
