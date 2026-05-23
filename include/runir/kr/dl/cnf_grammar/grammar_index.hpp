#ifndef RUNIR_CNF_GRAMMAR_GRAMMAR_INDEX_HPP_
#define RUNIR_CNF_GRAMMAR_GRAMMAR_INDEX_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <tyr/common/index_mixins.hpp>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::FamilyTag Family>
struct Index<runir::kr::dl::cnf_grammar::Grammar<Family>> : IndexMixin<Index<runir::kr::dl::cnf_grammar::Grammar<Family>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::cnf_grammar::Grammar<Family>>>;
    using Base::Base;
};

}

#endif
