#ifndef RUNIR_KR_PARSER_AST_ADAPTED_HPP_
#define RUNIR_KR_PARSER_AST_ADAPTED_HPP_

#include "runir/kr/parser/ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(runir::kr::parser::ast::Identifier, text)

#endif
