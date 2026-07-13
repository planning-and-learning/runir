#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_PARSER_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_PARSER_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/parser/config.hpp"

#include <string>

namespace runir::kr::dl::grammar::parser
{

template<runir::kr::dl::FamilyTag Family>
void parse_concept_ast(const std::string& description,
                       runir::kr::dl::grammar::ast::ConceptConstructor<Family>& result,
                       runir::kr::parser::ErrorHandlerType& error_handler);

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::ConceptConstructor<Family> parse_concept_ast(const std::string& description);

template<runir::kr::dl::FamilyTag Family>
void parse_role_ast(const std::string& description,
                    runir::kr::dl::grammar::ast::RoleConstructor<Family>& result,
                    runir::kr::parser::ErrorHandlerType& error_handler);

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::RoleConstructor<Family> parse_role_ast(const std::string& description);

template<runir::kr::dl::FamilyTag Family>
void parse_boolean_ast(const std::string& description,
                       runir::kr::dl::grammar::ast::BooleanConstructor<Family>& result,
                       runir::kr::parser::ErrorHandlerType& error_handler);

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::BooleanConstructor<Family> parse_boolean_ast(const std::string& description);

template<runir::kr::dl::FamilyTag Family>
void parse_numerical_ast(const std::string& description,
                         runir::kr::dl::grammar::ast::NumericalConstructor<Family>& result,
                         runir::kr::parser::ErrorHandlerType& error_handler);

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::NumericalConstructor<Family> parse_numerical_ast(const std::string& description);

template<runir::kr::dl::FamilyTag Family>
void parse_grammar_ast(const std::string& description,
                       runir::kr::dl::grammar::ast::Grammar<Family>& result,
                       runir::kr::parser::ErrorHandlerType& error_handler);

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::Grammar<Family> parse_grammar_ast(const std::string& description);

}  // namespace runir::kr::dl::grammar::parser

#endif
