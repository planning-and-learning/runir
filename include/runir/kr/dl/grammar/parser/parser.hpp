#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_PARSER_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_PARSER_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"

#include <string>

namespace runir::kr::dl::grammar::parser
{

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::ConceptConstructor<Family> parse_concept_ast(const std::string& description);

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::RoleConstructor<Family> parse_role_ast(const std::string& description);

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::BooleanConstructor<Family> parse_boolean_ast(const std::string& description);

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::NumericalConstructor<Family> parse_numerical_ast(const std::string& description);

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::Grammar<Family> parse_grammar_ast(const std::string& description);

}  // namespace runir::kr::dl::grammar::parser

#endif
