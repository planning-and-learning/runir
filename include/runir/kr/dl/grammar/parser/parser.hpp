#ifndef RUNIR_GRAMMAR_PARSER_PARSER_HPP_
#define RUNIR_GRAMMAR_PARSER_PARSER_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"

#include <string>

namespace runir::kr::dl::grammar::parser
{

template<runir::kr::dl::FamilyTag Family>
ast::Grammar<Family> parse_grammar_ast(const std::string& description);

}

#endif
