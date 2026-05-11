#ifndef RUNIR_GRAMMAR_BOOLEAN_INDEX_HPP_
#define RUNIR_GRAMMAR_BOOLEAN_INDEX_HPP_

#include "runir/kr/dl/grammar/constructors.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::BooleanConstructorTag Tag>
struct Index<runir::kr::dl::grammar::Boolean<Tag>> : IndexMixin<Index<runir::kr::dl::grammar::Boolean<Tag>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::grammar::Boolean<Tag>>>;
    using Base::Base;
};

}

#endif
