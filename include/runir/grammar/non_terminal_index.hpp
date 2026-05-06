#ifndef RUNIR_GRAMMAR_NON_TERMINAL_INDEX_HPP_
#define RUNIR_GRAMMAR_NON_TERMINAL_INDEX_HPP_

#include "runir/grammar/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::CategoryTag Category>
struct Index<runir::grammar::NonTerminal<Category>> : IndexMixin<Index<runir::grammar::NonTerminal<Category>>>
{
    using Base = IndexMixin<Index<runir::grammar::NonTerminal<Category>>>;
    using Base::Base;
};

}

#endif
