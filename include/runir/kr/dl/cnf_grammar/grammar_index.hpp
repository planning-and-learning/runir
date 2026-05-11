#ifndef RUNIR_CNF_GRAMMAR_GRAMMAR_INDEX_HPP_
#define RUNIR_CNF_GRAMMAR_GRAMMAR_INDEX_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Index<runir::kr::dl::cnf_grammar::GrammarTag> : IndexMixin<Index<runir::kr::dl::cnf_grammar::GrammarTag>>
{
    using Base = IndexMixin<Index<runir::kr::dl::cnf_grammar::GrammarTag>>;
    using Base::Base;
};

}

#endif
