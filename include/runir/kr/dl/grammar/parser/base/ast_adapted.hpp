#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_BASE_AST_ADAPTED_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_BASE_AST_ADAPTED_HPP_

#include "runir/kr/dl/grammar/parser/base/ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_TPL_STRUCT((Family) (Category), (runir::kr::dl::grammar::parser::base::ast::NonTerminal)(Family) (Category), (std::string, name))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::parser::base::ast::ConceptBot)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::parser::base::ast::ConceptTop)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::parser::base::ast::ConceptAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::parser::base::ast::ConceptAtomicGoal, predicate_name, polarity)
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptIntersection)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptChoice<Family>,
                               lhs)(runir::kr::dl::grammar::parser::base::ast::ConceptChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptUnion)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptChoice<Family>,
                               lhs)(runir::kr::dl::grammar::parser::base::ast::ConceptChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptNegation)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptChoice<Family>, arg))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptValueRestriction)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>,
                               lhs)(runir::kr::dl::grammar::parser::base::ast::ConceptChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptExistentialQuantification)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>,
                               lhs)(runir::kr::dl::grammar::parser::base::ast::ConceptChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptAtLeastNumberRestriction)(Family),
                              (ygg::uint_t, n)(runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>, role))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptAtMostNumberRestriction)(Family),
                              (ygg::uint_t, n)(runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>, role))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptExactNumberRestriction)(Family),
                              (ygg::uint_t, n)(runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>, role))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptQualifiedAtLeastNumberRestriction)(Family),
                              (ygg::uint_t, n)(runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>,
                                               role)(runir::kr::dl::grammar::parser::base::ast::ConceptChoice<Family>, concept_))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptQualifiedAtMostNumberRestriction)(Family),
                              (ygg::uint_t, n)(runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>,
                                               role)(runir::kr::dl::grammar::parser::base::ast::ConceptChoice<Family>, concept_))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptQualifiedExactNumberRestriction)(Family),
                              (ygg::uint_t, n)(runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>,
                                               role)(runir::kr::dl::grammar::parser::base::ast::ConceptChoice<Family>, concept_))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptRoleValueMap)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>,
                               lhs)(runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptAgreement)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>,
                               lhs)(runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptRoleFillers)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>, role)(std::vector<std::string>, object_names))
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::parser::base::ast::ConceptOneOf, object_names)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::parser::base::ast::ConceptNominal, object_name)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::parser::base::ast::RoleUniversal)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::parser::base::ast::RoleAtomicState, predicate_name)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::parser::base::ast::RoleAtomicGoal, predicate_name, polarity)
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleIntersection)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>,
                               lhs)(runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleUnion)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>,
                               lhs)(runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleComplement)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>, arg))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleInverse)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>, arg))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleComposition)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>,
                               lhs)(runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleTransitiveClosure)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>, arg))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleReflexiveTransitiveClosure)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>, arg))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleRestriction)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleChoice<Family>,
                               lhs)(runir::kr::dl::grammar::parser::base::ast::ConceptChoice<Family>, rhs))
BOOST_FUSION_ADAPT_TPL_STRUCT((Family),
                              (runir::kr::dl::grammar::parser::base::ast::RoleIdentity)(Family),
                              (runir::kr::dl::grammar::parser::base::ast::ConceptChoice<Family>, arg))

#endif
