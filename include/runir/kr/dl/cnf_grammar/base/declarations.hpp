#ifndef RUNIR_KR_DL_CNF_GRAMMAR_BASE_DECLARATIONS_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_BASE_DECLARATIONS_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"

namespace runir::kr::dl::cnf_grammar::base
{

using FamilyTag = runir::kr::dl::BaseFamilyTag;
using GrammarTag = runir::kr::dl::cnf_grammar::Grammar<FamilyTag>;

template<typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<FamilyTag, Tag>
using Concept = runir::kr::dl::cnf_grammar::Concept<FamilyTag, Tag>;

template<runir::kr::dl::CategoryTag Category>
using Constructor = runir::kr::dl::cnf_grammar::Constructor<FamilyTag, Category>;

}  // namespace runir::kr::dl::cnf_grammar::base

#endif
