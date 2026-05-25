#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_BASE_PARSER_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_BASE_PARSER_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/parser/base/ast.hpp"

#include <string>

namespace runir::kr::dl::grammar::parser::base
{

runir::kr::dl::grammar::ast::Grammar<runir::kr::BaseFamilyTag> parse_grammar_ast(const std::string& description);

ast::ConceptConstructor<runir::kr::BaseFamilyTag> parse_concept_ast(const std::string& description);
ast::RoleConstructor<runir::kr::BaseFamilyTag> parse_role_ast(const std::string& description);

}  // namespace runir::kr::dl::grammar::parser::base

#endif
