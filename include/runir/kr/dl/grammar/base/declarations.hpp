#ifndef RUNIR_KR_DL_GRAMMAR_BASE_DECLARATIONS_HPP_
#define RUNIR_KR_DL_GRAMMAR_BASE_DECLARATIONS_HPP_

#include "runir/kr/dl/grammar/declarations.hpp"

namespace runir::kr::dl::grammar::base
{

using FamilyTag = runir::kr::dl::BaseFamilyTag;

template<typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<FamilyTag, Tag>
using Concept = runir::kr::dl::grammar::Concept<FamilyTag, Tag>;

template<runir::kr::dl::RoleConstructorTag Tag>
using Role = runir::kr::dl::grammar::Role<FamilyTag, Tag>;

template<runir::kr::dl::BooleanConstructorTag Tag>
using Boolean = runir::kr::dl::grammar::Boolean<FamilyTag, Tag>;

template<runir::kr::dl::NumericalConstructorTag Tag>
using Numerical = runir::kr::dl::grammar::Numerical<FamilyTag, Tag>;

template<runir::kr::dl::CategoryTag Category>
using Constructor = runir::kr::dl::grammar::Constructor<FamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminal = runir::kr::dl::grammar::NonTerminal<FamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorOrNonTerminal = runir::kr::dl::grammar::ConstructorOrNonTerminal<FamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRule = runir::kr::dl::grammar::DerivationRule<FamilyTag, Category>;

using GrammarTag = runir::kr::dl::grammar::GrammarTag<FamilyTag>;

}  // namespace runir::kr::dl::grammar::base

#endif
