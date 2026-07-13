#ifndef RUNIR_KR_PS_BASE_DL_AST_AST_ADAPTED_HPP_
#define RUNIR_KR_PS_BASE_DL_AST_AST_ADAPTED_HPP_

#include "runir/kr/parser/ast_adapted.hpp"
#include "runir/kr/ps/base/dl/ast/ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::ps::base::dl::ast::Concept)(Family), symbol, feature)
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::ps::base::dl::ast::BooleanFeature)(Family), symbol, feature)
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::ps::base::dl::ast::NumericalFeature)(Family), symbol, feature)

BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::ps::base::dl::ast::Condition)(Family), observation, feature)
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::ps::base::dl::ast::Effect)(Family), observation, feature)
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::ps::base::dl::ast::Rule)(Family), symbol, conditions, effects)
BOOST_FUSION_ADAPT_TPL_STRUCT((Family), (runir::kr::ps::base::dl::ast::Sketch)(Family), features, rules)

#endif
