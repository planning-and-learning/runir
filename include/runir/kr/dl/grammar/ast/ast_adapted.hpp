#ifndef RUNIR_GRAMMAR_AST_AST_ADAPTED_HPP_
#define RUNIR_GRAMMAR_AST_AST_ADAPTED_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/parser/ast_adapted.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryAtomicState, predicate_name, columns)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryAtomicGoal, predicate_name, polarity, columns)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryConcept, columns, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryRole, columns, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryJoin, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryUnion, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryDifference, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryProject, columns, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryRename, columns, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QuerySelectEqual, lhs_column, rhs_column, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QuerySelectValue, column, object_name, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryGenericJoin, variable_order, output_order, args)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryEmpty, columns)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryMustPass, keys, sources, edges, targets, probes)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptProject, columns, arg)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleProject, columns, arg)

BOOST_FUSION_ADAPT_TPL_STRUCT((Category), (runir::kr::dl::grammar::ast::NonTerminal) (Category), name)
BOOST_FUSION_ADAPT_TPL_STRUCT((Category), (runir::kr::dl::grammar::ast::DerivationRule) (Category), lhs, rhs)

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptBot)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptTop)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptAtomicGoal, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptIntersection,
                          (runir::kr::dl::grammar::ast::ConceptChoice, lhs)(runir::kr::dl::grammar::ast::ConceptChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptUnion,
                          (runir::kr::dl::grammar::ast::ConceptChoice, lhs)(runir::kr::dl::grammar::ast::ConceptChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptNegation, (runir::kr::dl::grammar::ast::ConceptChoice, arg))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptValueRestriction,
                          (runir::kr::dl::grammar::ast::RoleChoice, lhs)(runir::kr::dl::grammar::ast::ConceptChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptExistentialQuantification,
                          (runir::kr::dl::grammar::ast::RoleChoice, lhs)(runir::kr::dl::grammar::ast::ConceptChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction, (ygg::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice, role))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction, (ygg::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice, role))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptExactNumberRestriction, (ygg::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice, role))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction,
                          (ygg::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice, role)(runir::kr::dl::grammar::ast::ConceptChoice, concept_))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction,
                          (ygg::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice, role)(runir::kr::dl::grammar::ast::ConceptChoice, concept_))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction,
                          (ygg::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice, role)(runir::kr::dl::grammar::ast::ConceptChoice, concept_))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptRoleValueMap,
                          (runir::kr::dl::grammar::ast::RoleChoice, lhs)(runir::kr::dl::grammar::ast::RoleChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptAgreement,
                          (runir::kr::dl::grammar::ast::RoleChoice, lhs)(runir::kr::dl::grammar::ast::RoleChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptRoleFillers, role, object_names)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptOneOf, object_names)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptNominal, object_name)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptRegister, reference)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptArgument, reference)

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleUniversal)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleAtomicGoal, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleIntersection,
                          (runir::kr::dl::grammar::ast::RoleChoice, lhs)(runir::kr::dl::grammar::ast::RoleChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleUnion, (runir::kr::dl::grammar::ast::RoleChoice, lhs)(runir::kr::dl::grammar::ast::RoleChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleComplement, (runir::kr::dl::grammar::ast::RoleChoice, arg))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleInverse, (runir::kr::dl::grammar::ast::RoleChoice, arg))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleComposition,
                          (runir::kr::dl::grammar::ast::RoleChoice, lhs)(runir::kr::dl::grammar::ast::RoleChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleTransitiveClosure, (runir::kr::dl::grammar::ast::RoleChoice, arg))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure, (runir::kr::dl::grammar::ast::RoleChoice, arg))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleRestriction,
                          (runir::kr::dl::grammar::ast::RoleChoice, lhs)(runir::kr::dl::grammar::ast::ConceptChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleIdentity, (runir::kr::dl::grammar::ast::ConceptChoice, arg))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleRegister, reference)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleArgument, reference)

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanAtomicState, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanAtomicGoal, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanNonempty, (runir::kr::dl::grammar::ast::ConstructorOrNonTerminalVariant, arg))

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalCount, (runir::kr::dl::grammar::ast::ConstructorOrNonTerminalVariant, arg))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalDistance,
                          (runir::kr::dl::grammar::ast::ConceptOrQueryVariant, lhs)(runir::kr::dl::grammar::ast::RoleOrQueryVariant,
                                                                                    mid)(runir::kr::dl::grammar::ast::ConceptOrQueryVariant, rhs))

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::Grammar, (std::vector<runir::kr::dl::grammar::ast::DerivationRuleVariant>, rules))

// Boolean comparisons (two boolean choices).
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanEq,
                          (runir::kr::dl::grammar::ast::BooleanChoice, lhs)(runir::kr::dl::grammar::ast::BooleanChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanNeq,
                          (runir::kr::dl::grammar::ast::BooleanChoice, lhs)(runir::kr::dl::grammar::ast::BooleanChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanLt,
                          (runir::kr::dl::grammar::ast::BooleanChoice, lhs)(runir::kr::dl::grammar::ast::BooleanChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanLe,
                          (runir::kr::dl::grammar::ast::BooleanChoice, lhs)(runir::kr::dl::grammar::ast::BooleanChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanGt,
                          (runir::kr::dl::grammar::ast::BooleanChoice, lhs)(runir::kr::dl::grammar::ast::BooleanChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanGe,
                          (runir::kr::dl::grammar::ast::BooleanChoice, lhs)(runir::kr::dl::grammar::ast::BooleanChoice, rhs))

// Numerical comparisons (two numerical choices) -> Boolean.
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalEq,
                          (runir::kr::dl::grammar::ast::NumericalChoice, lhs)(runir::kr::dl::grammar::ast::NumericalChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalNeq,
                          (runir::kr::dl::grammar::ast::NumericalChoice, lhs)(runir::kr::dl::grammar::ast::NumericalChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalLt,
                          (runir::kr::dl::grammar::ast::NumericalChoice, lhs)(runir::kr::dl::grammar::ast::NumericalChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalLe,
                          (runir::kr::dl::grammar::ast::NumericalChoice, lhs)(runir::kr::dl::grammar::ast::NumericalChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalGt,
                          (runir::kr::dl::grammar::ast::NumericalChoice, lhs)(runir::kr::dl::grammar::ast::NumericalChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalGe,
                          (runir::kr::dl::grammar::ast::NumericalChoice, lhs)(runir::kr::dl::grammar::ast::NumericalChoice, rhs))

// Constants.
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanConstant, (bool, value))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalConstant, (ygg::uint_t, value))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalArgument, reference)

// Numerical binary operators (two numerical choices) -> Numerical.
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalAdd,
                          (runir::kr::dl::grammar::ast::NumericalChoice, lhs)(runir::kr::dl::grammar::ast::NumericalChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalSub,
                          (runir::kr::dl::grammar::ast::NumericalChoice, lhs)(runir::kr::dl::grammar::ast::NumericalChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalMul,
                          (runir::kr::dl::grammar::ast::NumericalChoice, lhs)(runir::kr::dl::grammar::ast::NumericalChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalDiv,
                          (runir::kr::dl::grammar::ast::NumericalChoice, lhs)(runir::kr::dl::grammar::ast::NumericalChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalMin,
                          (runir::kr::dl::grammar::ast::NumericalChoice, lhs)(runir::kr::dl::grammar::ast::NumericalChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalMax,
                          (runir::kr::dl::grammar::ast::NumericalChoice, lhs)(runir::kr::dl::grammar::ast::NumericalChoice, rhs))

// Logical operators.
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanAnd,
                          (runir::kr::dl::grammar::ast::BooleanChoice, lhs)(runir::kr::dl::grammar::ast::BooleanChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanOr,
                          (runir::kr::dl::grammar::ast::BooleanChoice, lhs)(runir::kr::dl::grammar::ast::BooleanChoice, rhs))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanNot, (runir::kr::dl::grammar::ast::BooleanChoice, arg))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanArgument, reference)

#endif
