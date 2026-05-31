#ifndef RUNIR_CNF_GRAMMAR_SUBSTITUTION_RULE_INDEX_HPP_
#define RUNIR_CNF_GRAMMAR_SUBSTITUTION_RULE_INDEX_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <yggdrasil/ids/index_mixins.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Index<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, Category>> : IndexMixin<Index<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, Category>>>
{
    using Base = IndexMixin<Index<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, Category>>>;
    using Base::Base;
};

}

#endif
