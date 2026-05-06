#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_INDEX_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_INDEX_HPP_

#include "runir/grammar/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::CategoryTag Category>
struct Index<runir::grammar::Constructor<Category>> : IndexMixin<Index<runir::grammar::Constructor<Category>>>
{
    using Base = IndexMixin<Index<runir::grammar::Constructor<Category>>>;
    using Base::Base;
};

}

#endif
