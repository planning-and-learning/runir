#ifndef RUNIR_GRAMMAR_BOOLEAN_INDEX_HPP_
#define RUNIR_GRAMMAR_BOOLEAN_INDEX_HPP_

#include "runir/grammar/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::BooleanConstructorTag Tag>
struct Index<runir::grammar::Boolean<Tag>> : IndexMixin<Index<runir::grammar::Boolean<Tag>>>
{
    using Base = IndexMixin<Index<runir::grammar::Boolean<Tag>>>;
    using Base::Base;
};

}

#endif
