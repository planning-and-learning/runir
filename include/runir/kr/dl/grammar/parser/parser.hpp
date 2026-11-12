#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_PARSER_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_PARSER_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/parser/declarations.hpp"

#include <string>

namespace runir::kr::dl::grammar::parser
{

/// The full DL syntax.
struct FullDialectTag
{
};

template<typename Dialect = FullDialectTag>
void parse_concept_ast(const std::string& description,
                       runir::kr::dl::grammar::ast::ConceptConstructor& result,
                       runir::kr::parser::ErrorHandlerType& error_handler);

template<typename Dialect = FullDialectTag>
runir::kr::dl::grammar::ast::ConceptConstructor parse_concept_ast(const std::string& description);

template<typename Dialect = FullDialectTag>
void parse_role_ast(const std::string& description, runir::kr::dl::grammar::ast::RoleConstructor& result, runir::kr::parser::ErrorHandlerType& error_handler);

template<typename Dialect = FullDialectTag>
runir::kr::dl::grammar::ast::RoleConstructor parse_role_ast(const std::string& description);

template<typename Dialect = FullDialectTag>
void parse_boolean_ast(const std::string& description,
                       runir::kr::dl::grammar::ast::BooleanConstructor& result,
                       runir::kr::parser::ErrorHandlerType& error_handler);

template<typename Dialect = FullDialectTag>
runir::kr::dl::grammar::ast::BooleanConstructor parse_boolean_ast(const std::string& description);

template<typename Dialect = FullDialectTag>
void parse_numerical_ast(const std::string& description,
                         runir::kr::dl::grammar::ast::NumericalConstructor& result,
                         runir::kr::parser::ErrorHandlerType& error_handler);

template<typename Dialect = FullDialectTag>
runir::kr::dl::grammar::ast::NumericalConstructor parse_numerical_ast(const std::string& description);

template<typename Dialect = FullDialectTag>
void parse_grammar_ast(const std::string& description, runir::kr::dl::grammar::ast::Grammar& result, runir::kr::parser::ErrorHandlerType& error_handler);

template<typename Dialect = FullDialectTag>
runir::kr::dl::grammar::ast::Grammar parse_grammar_ast(const std::string& description);

}  // namespace runir::kr::dl::grammar::parser

#endif
