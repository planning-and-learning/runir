#ifndef RUNIR_CNF_GRAMMAR_NON_TERMINAL_INDEX_HPP_
#define RUNIR_CNF_GRAMMAR_NON_TERMINAL_INDEX_HPP_

#include "runir/cnf_grammar/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::CategoryTag Category>
struct Index<runir::cnf_grammar::NonTerminal<Category>> : IndexMixin<Index<runir::cnf_grammar::NonTerminal<Category>>>
{
    using Base = IndexMixin<Index<runir::cnf_grammar::NonTerminal<Category>>>;
    using Base::Base;
};

}

#endif
