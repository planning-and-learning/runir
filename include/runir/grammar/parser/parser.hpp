#ifndef RUNIR_GRAMMAR_PARSER_PARSER_HPP_
#define RUNIR_GRAMMAR_PARSER_PARSER_HPP_

#include "runir/grammar/ast/ast.hpp"

#include <string>

namespace runir::grammar::parser
{

ast::Grammar parse_grammar_ast(const std::string& description);

}

#endif
