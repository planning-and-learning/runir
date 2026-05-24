#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_EXT_PARSER_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_EXT_PARSER_HPP_

#include "runir/kr/dl/grammar/parser/ext/ast.hpp"

#include <string>

namespace runir::kr::dl::grammar::parser::ext
{

runir::kr::dl::grammar::parser::base::ast::ConceptConstructor<runir::kr::dl::ExtFamilyTag> parse_concept_ast(const std::string& description);
runir::kr::dl::grammar::parser::base::ast::RoleConstructor<runir::kr::dl::ExtFamilyTag> parse_role_ast(const std::string& description);

}  // namespace runir::kr::dl::grammar::parser::ext

#endif
