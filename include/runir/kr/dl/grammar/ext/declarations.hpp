#ifndef RUNIR_KR_DL_GRAMMAR_EXT_DECLARATIONS_HPP_
#define RUNIR_KR_DL_GRAMMAR_EXT_DECLARATIONS_HPP_

#include "runir/kr/dl/ext/register_index.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"

namespace runir::kr::dl::grammar::ext
{

using FamilyTag = runir::kr::dl::ExtFamilyTag;
using RegisterTag = runir::kr::dl::RegisterTag;

template<typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<FamilyTag, Tag>
using Concept = runir::kr::dl::grammar::Concept<FamilyTag, Tag>;

template<runir::kr::dl::CategoryTag Category>
using Constructor = runir::kr::dl::grammar::Constructor<FamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorOrNonTerminal = runir::kr::dl::grammar::ConstructorOrNonTerminal<FamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminal = runir::kr::dl::grammar::NonTerminal<FamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRule = runir::kr::dl::grammar::DerivationRule<FamilyTag, Category>;

using GrammarTag = runir::kr::dl::grammar::GrammarTag<FamilyTag>;

}  // namespace runir::kr::dl::grammar::ext

#endif
