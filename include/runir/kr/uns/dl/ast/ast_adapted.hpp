#ifndef RUNIR_KR_UNS_DL_AST_AST_ADAPTED_HPP_
#define RUNIR_KR_UNS_DL_AST_AST_ADAPTED_HPP_

#include "runir/kr/uns/dl/ast/ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(runir::kr::uns::dl::ast::BooleanFeature, symbol, feature)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::uns::dl::ast::PositiveLiteral, symbol)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::uns::dl::ast::NegativeLiteral, symbol)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::uns::dl::ast::Conjunction, literals)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::uns::dl::ast::Disjunction, clauses)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::uns::dl::ast::Classifier, symbol, features, expression)

#endif
