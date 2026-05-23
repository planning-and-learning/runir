#ifndef RUNIR_GRAMMAR_AST_AST_ADAPTED_HPP_
#define RUNIR_GRAMMAR_AST_AST_ADAPTED_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_TPL_STRUCT((Family) (Category), (runir::kr::dl::grammar::ast::NonTerminal)(Family) (Category), (std::string, name))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family) (Category), (runir::kr::dl::grammar::ast::DerivationRule)(Family) (Category), lhs, rhs)

BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::ConceptBot)(Family))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::ConceptTop)(Family))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::ConceptAtomicState)(Family), (std::string, predicate_name))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::ConceptAtomicGoal)(Family), (std::string, predicate_name)(bool, polarity))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::ConceptIntersection)(Family),
                              (runir::kr::dl::grammar::ast::ConceptChoice<Family>, lhs)(runir::kr::dl::grammar::ast::ConceptChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::ConceptUnion)(Family),
                              (runir::kr::dl::grammar::ast::ConceptChoice<Family>, lhs)(runir::kr::dl::grammar::ast::ConceptChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::ConceptNegation)(Family), (runir::kr::dl::grammar::ast::ConceptChoice<Family>, arg))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::ConceptValueRestriction)(Family),
                              (runir::kr::dl::grammar::ast::RoleChoice<Family>, lhs)(runir::kr::dl::grammar::ast::ConceptChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::ConceptExistentialQuantification)(Family),
                              (runir::kr::dl::grammar::ast::RoleChoice<Family>, lhs)(runir::kr::dl::grammar::ast::ConceptChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction)(Family),
                              (tyr::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice<Family>, role))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction)(Family),
                              (tyr::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice<Family>, role))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::ConceptExactNumberRestriction)(Family),
                              (tyr::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice<Family>, role))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction)(Family),
                              (tyr::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice<Family>, role)(runir::kr::dl::grammar::ast::ConceptChoice<Family>,
                                                                                                      concept_))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction)(Family),
                              (tyr::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice<Family>, role)(runir::kr::dl::grammar::ast::ConceptChoice<Family>,
                                                                                                      concept_))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction)(Family),
                              (tyr::uint_t, n)(runir::kr::dl::grammar::ast::RoleChoice<Family>, role)(runir::kr::dl::grammar::ast::ConceptChoice<Family>,
                                                                                                      concept_))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::ConceptRoleValueMap)(Family),
                              (runir::kr::dl::grammar::ast::RoleChoice<Family>, lhs)(runir::kr::dl::grammar::ast::RoleChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::ConceptAgreement)(Family),
                              (runir::kr::dl::grammar::ast::RoleChoice<Family>, lhs)(runir::kr::dl::grammar::ast::RoleChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::ConceptRoleFillers)(Family),
                              (runir::kr::dl::grammar::ast::RoleChoice<Family>, role)(std::vector<std::string>, object_names))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::ConceptOneOf)(Family), (std::vector<std::string>, object_names))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::ConceptNominal)(Family), (std::string, object_name))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::ConceptRegister)(Family), (tyr::uint_t, identifier))

BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::RoleUniversal)(Family))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::RoleAtomicState)(Family), (std::string, predicate_name))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::RoleAtomicGoal)(Family), (std::string, predicate_name)(bool, polarity))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::RoleIntersection)(Family),
                              (runir::kr::dl::grammar::ast::RoleChoice<Family>, lhs)(runir::kr::dl::grammar::ast::RoleChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::RoleUnion)(Family),
                              (runir::kr::dl::grammar::ast::RoleChoice<Family>, lhs)(runir::kr::dl::grammar::ast::RoleChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::RoleComplement)(Family), (runir::kr::dl::grammar::ast::RoleChoice<Family>, arg))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::RoleInverse)(Family), (runir::kr::dl::grammar::ast::RoleChoice<Family>, arg))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::RoleComposition)(Family),
                              (runir::kr::dl::grammar::ast::RoleChoice<Family>, lhs)(runir::kr::dl::grammar::ast::RoleChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::RoleTransitiveClosure)(Family), (runir::kr::dl::grammar::ast::RoleChoice<Family>, arg))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure)(Family),
                              (runir::kr::dl::grammar::ast::RoleChoice<Family>, arg))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::RoleRestriction)(Family),
                              (runir::kr::dl::grammar::ast::RoleChoice<Family>, lhs)(runir::kr::dl::grammar::ast::ConceptChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::RoleIdentity)(Family), (runir::kr::dl::grammar::ast::ConceptChoice<Family>, arg))

BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::BooleanAtomicState)(Family), (std::string, predicate_name)(bool, polarity))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::dl::grammar::ast::BooleanAtomicGoal)(Family), (std::string, predicate_name)(bool, polarity))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::BooleanNonempty)(Family),
                              (runir::kr::dl::grammar::ast::ConstructorOrNonTerminalVariant<Family>, arg))

BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::NumericalCount)(Family),
                              (runir::kr::dl::grammar::ast::ConstructorOrNonTerminalVariant<Family>, arg))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::NumericalDistance)(Family),
                              (runir::kr::dl::grammar::ast::ConceptChoice<Family>, lhs)(runir::kr::dl::grammar::ast::RoleChoice<Family>,
                                                                                        mid)(runir::kr::dl::grammar::ast::ConceptChoice<Family>, rhs))

BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::ast::Grammar)(Family),
                              (std::vector<runir::kr::dl::grammar::ast::DerivationRuleVariant<Family>>, rules))

#endif
