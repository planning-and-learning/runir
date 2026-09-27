#ifndef RUNIR_KR_PS_ICP_DL_AST_AST_ADAPTED_HPP_
#define RUNIR_KR_PS_ICP_DL_AST_AST_ADAPTED_HPP_

#include "runir/kr/ps/ext/dl/ast/ast_adapted.hpp"
#include "runir/kr/ps/icp/dl/ast/ast.hpp"

BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::icp::dl::ast::ArgumentObject, name)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::icp::dl::ast::RegisterObject, name)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::icp::dl::ast::XCondition, belongs, object, feature)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::icp::dl::ast::XEffect, enter, object, feature)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::icp::dl::ast::Crule, action, arguments, conditions, xconditions, xeffects, effects)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::icp::dl::ast::RuleEntry, symbol, source, target, rules)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::icp::dl::ast::ResetPair, before, after)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::icp::dl::ast::Module, name, registers, entry, memory_states, features, rule_entries, reset_pairs)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::icp::dl::ast::Program, entry, module)

#endif
