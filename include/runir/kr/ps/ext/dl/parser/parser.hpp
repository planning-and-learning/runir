#ifndef RUNIR_KR_PS_EXT_DL_PARSER_PARSER_HPP_
#define RUNIR_KR_PS_EXT_DL_PARSER_PARSER_HPP_

#include "runir/kr/parser/declarations.hpp"
#include "runir/kr/ps/ext/dl/ast/ast_adapted.hpp"

#include <string>
#include <vector>

namespace runir::kr::ps::ext::dl::parser
{

ast::Module parse_module_ast(const std::string& description);
ast::Program parse_program_ast(const std::string& description);
void parse_module_ast(const std::string& description, ast::Module& result, runir::kr::parser::ErrorHandlerType& error_handler);
void parse_program_ast(const std::string& description, ast::Program& result, runir::kr::parser::ErrorHandlerType& error_handler);

}  // namespace runir::kr::ps::ext::dl::parser

#endif
