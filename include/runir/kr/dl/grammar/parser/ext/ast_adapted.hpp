#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_EXT_AST_ADAPTED_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_EXT_AST_ADAPTED_HPP_

#include "runir/kr/dl/grammar/parser/ext/ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::parser::ext::ast::ConceptRegister, identifier)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::dl::grammar::parser::ext::ast::RoleRegister, identifier)
BOOST_FUSION_ADAPT_TPL_STRUCT((Category), (runir::kr::dl::grammar::parser::ext::ast::Argument)(Category), (tyr::uint_t, identifier))

#endif
