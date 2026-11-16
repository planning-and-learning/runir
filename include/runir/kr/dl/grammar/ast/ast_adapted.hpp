#ifndef RUNIR_GRAMMAR_AST_AST_ADAPTED_HPP_
#define RUNIR_GRAMMAR_AST_AST_ADAPTED_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/parser/ast_adapted.hpp"

#include <boost/fusion/include/adapt_struct.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryAtomicState, predicate_name, columns)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryAtomicGoal, predicate_name, polarity, columns)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryConcept, columns, operand)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryRole, columns, operand)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryJoin, left_operand, right_operand)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryUnion, left_operand, right_operand)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryDifference, left_operand, right_operand)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryProject, columns, operand)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryRename, columns, operand)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QuerySelectEqual, left_column, right_column, operand)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QuerySelectValue, column, object_name, operand)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryGenericJoin, variable_order, output_order, args)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryEmpty, columns)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::QueryMustPass, keys, sources, edges, targets, probes)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptProject, columns, operand)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleProject, columns, operand)

BOOST_FUSION_ADAPT_TPL_STRUCT((Category), (runir::kr::dl::grammar::ast::NonTerminal) (Category), name)
BOOST_FUSION_ADAPT_TPL_STRUCT((Category), (runir::kr::dl::grammar::ast::DerivationRule) (Category), lhs, rhs)

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptBot)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptTop)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptAtomicGoal, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptIntersection,
                          (runir::kr::dl::grammar::ast::ConceptChoice, left_operand)(runir::kr::dl::grammar::ast::ConceptChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptUnion,
                          (runir::kr::dl::grammar::ast::ConceptChoice, left_operand)(runir::kr::dl::grammar::ast::ConceptChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptNegation, (runir::kr::dl::grammar::ast::ConceptChoice, operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptValueRestriction,
                          (runir::kr::dl::grammar::ast::RoleChoice, role)(runir::kr::dl::grammar::ast::ConceptChoice, qualifier))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptExistentialQuantification,
                          (runir::kr::dl::grammar::ast::RoleChoice, role)(runir::kr::dl::grammar::ast::ConceptChoice, qualifier))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction, (ygg::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice, role))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction, (ygg::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice, role))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptExactNumberRestriction, (ygg::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice, role))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction,
                          (ygg::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice, role)(runir::kr::dl::grammar::ast::ConceptChoice, qualifier))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction,
                          (ygg::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice, role)(runir::kr::dl::grammar::ast::ConceptChoice, qualifier))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction,
                          (ygg::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice, role)(runir::kr::dl::grammar::ast::ConceptChoice, qualifier))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptRoleValueMap,
                          (runir::kr::dl::grammar::ast::RoleChoice, left_operand)(runir::kr::dl::grammar::ast::RoleChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptAgreement,
                          (runir::kr::dl::grammar::ast::RoleChoice, left_operand)(runir::kr::dl::grammar::ast::RoleChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptRoleFillers, role, object_names)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptOneOf, object_names)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptNominal, object_name)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptRegister, slot)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::ConceptArgument, argument)

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleUniversal)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleAtomicGoal, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleIntersection,
                          (runir::kr::dl::grammar::ast::RoleChoice, left_operand)(runir::kr::dl::grammar::ast::RoleChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleUnion,
                          (runir::kr::dl::grammar::ast::RoleChoice, left_operand)(runir::kr::dl::grammar::ast::RoleChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleComplement, (runir::kr::dl::grammar::ast::RoleChoice, operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleInverse, (runir::kr::dl::grammar::ast::RoleChoice, operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleComposition,
                          (runir::kr::dl::grammar::ast::RoleChoice, left_operand)(runir::kr::dl::grammar::ast::RoleChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleTransitiveClosure, (runir::kr::dl::grammar::ast::RoleChoice, operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure, (runir::kr::dl::grammar::ast::RoleChoice, operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleRestriction,
                          (runir::kr::dl::grammar::ast::RoleChoice, role)(runir::kr::dl::grammar::ast::ConceptChoice, qualifier))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleIdentity, (runir::kr::dl::grammar::ast::ConceptChoice, operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleRegister, slot)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::RoleArgument, argument)

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanAtomicState, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanAtomicGoal, predicate_name, polarity)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanNonempty, (runir::kr::dl::grammar::ast::ConstructorOrNonTerminalVariant, operand))

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalCount, (runir::kr::dl::grammar::ast::ConstructorOrNonTerminalVariant, operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalDistance,
                          (runir::kr::dl::grammar::ast::ConceptOrQueryVariant, sources)(runir::kr::dl::grammar::ast::RoleOrQueryVariant,
                                                                                        edges)(runir::kr::dl::grammar::ast::ConceptOrQueryVariant, targets))

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::Grammar, (urd::Store<urd::sequence<runir::kr::dl::grammar::ast::DerivationRuleVariant>>, rules))

// Boolean comparisons (two boolean choices).
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanEq,
                          (runir::kr::dl::grammar::ast::BooleanChoice, left_operand)(runir::kr::dl::grammar::ast::BooleanChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanNeq,
                          (runir::kr::dl::grammar::ast::BooleanChoice, left_operand)(runir::kr::dl::grammar::ast::BooleanChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanLt,
                          (runir::kr::dl::grammar::ast::BooleanChoice, left_operand)(runir::kr::dl::grammar::ast::BooleanChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanLe,
                          (runir::kr::dl::grammar::ast::BooleanChoice, left_operand)(runir::kr::dl::grammar::ast::BooleanChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanGt,
                          (runir::kr::dl::grammar::ast::BooleanChoice, left_operand)(runir::kr::dl::grammar::ast::BooleanChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanGe,
                          (runir::kr::dl::grammar::ast::BooleanChoice, left_operand)(runir::kr::dl::grammar::ast::BooleanChoice, right_operand))

// Numerical comparisons (two numerical choices) -> Boolean.
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalEq,
                          (runir::kr::dl::grammar::ast::NumericalChoice, left_operand)(runir::kr::dl::grammar::ast::NumericalChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalNeq,
                          (runir::kr::dl::grammar::ast::NumericalChoice, left_operand)(runir::kr::dl::grammar::ast::NumericalChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalLt,
                          (runir::kr::dl::grammar::ast::NumericalChoice, left_operand)(runir::kr::dl::grammar::ast::NumericalChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalLe,
                          (runir::kr::dl::grammar::ast::NumericalChoice, left_operand)(runir::kr::dl::grammar::ast::NumericalChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalGt,
                          (runir::kr::dl::grammar::ast::NumericalChoice, left_operand)(runir::kr::dl::grammar::ast::NumericalChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalGe,
                          (runir::kr::dl::grammar::ast::NumericalChoice, left_operand)(runir::kr::dl::grammar::ast::NumericalChoice, right_operand))

// Constants.
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanConstant, (bool, value))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalConstant, (ygg::uint_t, value))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalArgument, argument)

// Numerical binary operators (two numerical choices) -> Numerical.
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalAdd,
                          (runir::kr::dl::grammar::ast::NumericalChoice, left_operand)(runir::kr::dl::grammar::ast::NumericalChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalSub,
                          (runir::kr::dl::grammar::ast::NumericalChoice, left_operand)(runir::kr::dl::grammar::ast::NumericalChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalMul,
                          (runir::kr::dl::grammar::ast::NumericalChoice, left_operand)(runir::kr::dl::grammar::ast::NumericalChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalDiv,
                          (runir::kr::dl::grammar::ast::NumericalChoice, left_operand)(runir::kr::dl::grammar::ast::NumericalChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalMin,
                          (runir::kr::dl::grammar::ast::NumericalChoice, left_operand)(runir::kr::dl::grammar::ast::NumericalChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::NumericalMax,
                          (runir::kr::dl::grammar::ast::NumericalChoice, left_operand)(runir::kr::dl::grammar::ast::NumericalChoice, right_operand))

// Logical operators.
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanAnd,
                          (runir::kr::dl::grammar::ast::BooleanChoice, left_operand)(runir::kr::dl::grammar::ast::BooleanChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanOr,
                          (runir::kr::dl::grammar::ast::BooleanChoice, left_operand)(runir::kr::dl::grammar::ast::BooleanChoice, right_operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanNot, (runir::kr::dl::grammar::ast::BooleanChoice, operand))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::ast::BooleanArgument, argument)

#endif
