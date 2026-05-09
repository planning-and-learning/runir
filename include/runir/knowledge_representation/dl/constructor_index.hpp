#ifndef RUNIR_CONSTRUCTOR_INDEX_HPP_
#define RUNIR_CONSTRUCTOR_INDEX_HPP_

#include "runir/knowledge_representation/dl/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::dl::Constructor<Category>> : IndexMixin<Index<runir::kr::dl::Constructor<Category>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::Constructor<Category>>>;
    using Base::Base;
};

}

#endif
