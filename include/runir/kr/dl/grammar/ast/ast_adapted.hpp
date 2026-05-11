#ifndef RUNIR_GRAMMAR_AST_AST_ADAPTED_HPP_
#define RUNIR_GRAMMAR_AST_AST_ADAPTED_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_TPL_STRUCT((D), (runir::kr::dl::grammar::ast::NonTerminal)(D), (std::string, name))
BOOST_FUSION_ADAPT_TPL_STRUCT((D),
                              (runir::kr::dl::grammar::ast::DerivationRule)(D),
                              (runir::kr::dl::grammar::ast::NonTerminal<D>, lhs)(std::vector<runir::kr::dl::grammar::ast::ConstructorOrNonTerminal<D>>, rhs))

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptBot)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptTop)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptAtomicGoal, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptIntersection, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptUnion, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptNegation, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptValueRestriction, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptExistentialQuantification, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptRoleValueMapContainment, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptRoleValueMapEquality, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptNominal, object_name)

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleUniversal)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleAtomicGoal, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleIntersection, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleUnion, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleComplement, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleInverse, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleComposition, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleTransitiveClosure, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleRestriction, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleIdentity, arg)

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanAtomicState, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanNonempty, arg)

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalCount, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalDistance, lhs, mid, rhs)

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::GrammarHead, concept_start, role_start, boolean_start, numerical_start)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::GrammarBody, rules)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::Grammar, head, body)

#endif
