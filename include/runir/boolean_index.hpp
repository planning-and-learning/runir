#ifndef RUNIR_BOOLEAN_INDEX_HPP_
#define RUNIR_BOOLEAN_INDEX_HPP_

#include "runir/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::BooleanConstructorTag Tag>
struct Index<runir::Boolean<Tag>> : IndexMixin<Index<runir::Boolean<Tag>>>
{
    using Base = IndexMixin<Index<runir::Boolean<Tag>>>;
    using Base::Base;
};

}

#endif
