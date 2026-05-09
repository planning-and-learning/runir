#ifndef RUNIR_GRAMMAR_PARSER_PARSER_HPP_
#define RUNIR_GRAMMAR_PARSER_PARSER_HPP_

#include "runir/knowledge_representation/dl/grammar/ast/ast.hpp"

#include <string>

namespace runir::kr::dl::grammar::parser
{

ast::Grammar parse_grammar_ast(const std::string& description);

}

#endif
