#ifndef RUNIR_KR_PS_BASE_DL_AST_AST_ADAPTED_HPP_
#define RUNIR_KR_PS_BASE_DL_AST_AST_ADAPTED_HPP_

#include "runir/kr/ps/base/dl/ast/ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::base::dl::ast::BooleanFeature, name, symbol, description, feature)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::base::dl::ast::NumericalFeature, name, symbol, description, feature)

BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::base::dl::ast::Condition, observation, feature)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::base::dl::ast::Effect, observation, feature)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::base::dl::ast::Rule, conditions, effects)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::base::dl::ast::Sketch, features, rules)

#endif
