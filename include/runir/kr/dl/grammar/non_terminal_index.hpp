#ifndef RUNIR_GRAMMAR_NON_TERMINAL_INDEX_HPP_
#define RUNIR_GRAMMAR_NON_TERMINAL_INDEX_HPP_

#include "runir/kr/dl/grammar/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::dl::grammar::NonTerminal<Family, Category>> : IndexMixin<Index<runir::kr::dl::grammar::NonTerminal<Family, Category>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::grammar::NonTerminal<Family, Category>>>;
    using Base::Base;
};

}  // namespace ygg

#endif
