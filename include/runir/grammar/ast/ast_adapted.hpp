#ifndef RUNIR_GRAMMAR_AST_AST_ADAPTED_HPP_
#define RUNIR_GRAMMAR_AST_AST_ADAPTED_HPP_

#include "runir/grammar/ast/ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_TPL_STRUCT((D), (runir::grammar::ast::NonTerminal)(D), (std::string, name))
BOOST_FUSION_ADAPT_TPL_STRUCT((D),
                              (runir::grammar::ast::DerivationRule)(D),
                              (runir::grammar::ast::NonTerminal<D>, lhs)(std::vector<runir::grammar::ast::ConstructorOrNonTerminal<D>>, rhs))

BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::ConceptBot)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::ConceptTop)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::ConceptAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::ConceptAtomicGoal, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::ConceptIntersection, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::ConceptUnion, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::ConceptNegation, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::ConceptValueRestriction, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::ConceptExistentialQuantification, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::ConceptRoleValueMapContainment, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::ConceptRoleValueMapEquality, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::ConceptNominal, object_name)

BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::RoleUniversal)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::RoleAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::RoleAtomicGoal, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::RoleIntersection, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::RoleUnion, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::RoleComplement, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::RoleInverse, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::RoleComposition, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::RoleTransitiveClosure, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::RoleReflexiveTransitiveClosure, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::RoleRestriction, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::RoleIdentity, arg)

BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::BooleanAtomicState, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::BooleanNonempty, arg)

BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::NumericalCount, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::NumericalDistance, lhs, mid, rhs)

BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::GrammarHead, concept_start, role_start, boolean_start, numerical_start)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::GrammarBody, rules)
BOOST_FUSION_ADAPT_STRUCT(runir::grammar::ast::Grammar, head, body)

#endif
