#ifndef RUNIR_KR_GP_DL_AST_AST_ADAPTED_HPP_
#define RUNIR_KR_GP_DL_AST_AST_ADAPTED_HPP_

#include "runir/kr/gp/dl/ast/ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(runir::kr::gp::dl::ast::BooleanFeature, name, symbol, description, feature)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::gp::dl::ast::NumericalFeature, name, symbol, description, feature)

BOOST_FUSION_ADAPT_STRUCT(runir::kr::gp::dl::ast::Condition, observation, feature)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::gp::dl::ast::Effect, observation, feature)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::gp::dl::ast::Rule, conditions, effects)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::gp::dl::ast::Policy, features, rules)

#endif
