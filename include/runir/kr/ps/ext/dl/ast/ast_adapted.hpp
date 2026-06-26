#ifndef RUNIR_KR_PS_EXT_DL_AST_AST_ADAPTED_HPP_
#define RUNIR_KR_PS_EXT_DL_AST_AST_ADAPTED_HPP_

#include "runir/kr/ps/ext/dl/ast/ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_TPL_STRUCT((Category), (runir::kr::ps::ext::dl::ast::Argument)(Category), symbol)
BOOST_FUSION_ADAPT_TPL_STRUCT((Category), (runir::kr::ps::ext::dl::ast::Register)(Category), symbol)
BOOST_FUSION_ADAPT_TPL_STRUCT((Category), (runir::kr::ps::ext::dl::ast::Feature)(Category), symbol, expression)

BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::ext::dl::ast::SymbolExpression, symbol)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::ext::dl::ast::ConstructorExpression, text)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::ext::dl::ast::NamedValue, value)

BOOST_FUSION_ADAPT_TPL_STRUCT((Category), (runir::kr::ps::ext::dl::ast::LoadRule)(Category), conditions, expression, reg)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::ext::dl::ast::SketchRule, conditions, effects)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::ext::dl::ast::DoRule, conditions, action, arguments, effects)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::ext::dl::ast::CallRule, conditions, callee, arguments)

BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::ext::dl::ast::RuleEntry, symbol, source, target, rules)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::ext::dl::ast::Module, name, arguments, registers, entry, memory_states, features, rule_entries)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::ps::ext::dl::ast::ModuleProgram, entry, modules)

#endif
