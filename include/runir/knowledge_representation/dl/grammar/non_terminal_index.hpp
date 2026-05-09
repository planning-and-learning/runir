#ifndef RUNIR_GRAMMAR_NON_TERMINAL_INDEX_HPP_
#define RUNIR_GRAMMAR_NON_TERMINAL_INDEX_HPP_

#include "runir/knowledge_representation/dl/grammar/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::dl::grammar::NonTerminal<Category>> : IndexMixin<Index<runir::kr::dl::grammar::NonTerminal<Category>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::grammar::NonTerminal<Category>>>;
    using Base::Base;
};

}

#endif
