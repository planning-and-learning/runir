#ifndef RUNIR_CONSTRUCTOR_INDEX_HPP_
#define RUNIR_CONSTRUCTOR_INDEX_HPP_

#include "runir/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::CategoryTag Category>
struct Index<runir::Constructor<Category>> : IndexMixin<Index<runir::Constructor<Category>>>
{
    using Base = IndexMixin<Index<runir::Constructor<Category>>>;
    using Base::Base;
};

}

#endif
