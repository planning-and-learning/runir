#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_OR_NON_TERMINAL_INDEX_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_OR_NON_TERMINAL_INDEX_HPP_

#include "runir/kr/dl/grammar/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<Category>> : IndexMixin<Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<Category>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<Category>>>;
    using Base::Base;
};

}

#endif
