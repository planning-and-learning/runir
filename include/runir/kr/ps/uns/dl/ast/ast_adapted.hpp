#ifndef RUNIR_KR_PS_UNS_DL_AST_AST_ADAPTED_HPP_
#define RUNIR_KR_PS_UNS_DL_AST_AST_ADAPTED_HPP_

#include "runir/kr/ps/uns/dl/ast/ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::ps::uns::dl::ast::Concept)(Family), symbol, description, feature)
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::ps::uns::dl::ast::BooleanFeature)(Family), symbol, description, feature)
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::ps::uns::dl::ast::NumericalFeature)(Family), symbol, description, feature)

BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::ps::uns::dl::ast::Condition)(Family), observation, feature)
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::ps::uns::dl::ast::Effect)(Family), observation, feature)
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::ps::uns::dl::ast::Rule)(Family), symbol, description, conditions, effects)
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::ps::uns::dl::ast::Sketch)(Family), features, rules)

#endif
