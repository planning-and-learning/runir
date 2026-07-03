#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_UNS_PARSER_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_UNS_PARSER_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"

#include <string>

namespace runir::kr::dl::grammar::parser::uns
{

runir::kr::dl::grammar::ast::Grammar<runir::kr::UnsFamilyTag> parse_grammar_ast(const std::string& description);

runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::ConceptTag> parse_concept_ast(const std::string& description);
runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::RoleTag> parse_role_ast(const std::string& description);
runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag> parse_boolean_ast(const std::string& description);
runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag> parse_numerical_ast(const std::string& description);

}  // namespace runir::kr::dl::grammar::parser::uns

#endif
