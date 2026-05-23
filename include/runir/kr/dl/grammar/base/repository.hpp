#ifndef RUNIR_KR_DL_GRAMMAR_BASE_REPOSITORY_HPP_
#define RUNIR_KR_DL_GRAMMAR_BASE_REPOSITORY_HPP_

#include "runir/kr/dl/grammar/base/datas.hpp"

#include <tyr/common/type_list.hpp>

namespace runir::kr::dl::grammar::base
{

template<runir::kr::dl::FamilyTag Family>
using ConceptTypes = tyr::MapTypeListSecondT<Concept, Family, runir::kr::dl::ConceptConstructorTags>;

template<runir::kr::dl::FamilyTag Family>
using RoleTypes = tyr::MapTypeListSecondT<Role, Family, runir::kr::dl::RoleConstructorTags>;

template<runir::kr::dl::FamilyTag Family>
using BooleanTypes = tyr::MapTypeListSecondT<Boolean, Family, runir::kr::dl::BooleanConstructorTags>;

template<runir::kr::dl::FamilyTag Family>
using NumericalTypes = tyr::MapTypeListSecondT<Numerical, Family, runir::kr::dl::NumericalConstructorTags>;

template<runir::kr::dl::FamilyTag Family>
using ConstructorTypes = tyr::MapTypeListSecondT<Constructor, Family, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using ConstructorOrNonTerminalTypes = tyr::MapTypeListSecondT<ConstructorOrNonTerminal, Family, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using NonTerminalTypes = tyr::MapTypeListSecondT<NonTerminal, Family, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using DerivationRuleTypes = tyr::MapTypeListSecondT<DerivationRule, Family, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using GrammarTypes = tyr::TypeList<GrammarTag<Family>>;

template<runir::kr::dl::FamilyTag Family>
using ConstructorRepositoryTypes = tyr::ConcatTypeListsT<ConceptTypes<Family>,
                                                         RoleTypes<Family>,
                                                         BooleanTypes<Family>,
                                                         NumericalTypes<Family>,
                                                         ConstructorTypes<Family>,
                                                         ConstructorOrNonTerminalTypes<Family>,
                                                         NonTerminalTypes<Family>,
                                                         DerivationRuleTypes<Family>,
                                                         GrammarTypes<Family>>;

}  // namespace runir::kr::dl::grammar::base

#endif
