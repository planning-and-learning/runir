#include "runir/kr/ps/ext/dl/parser/parser.hpp"

#include "runir/kr/parser/diagnostics.hpp"
#include "runir/kr/parser/parser.hpp"
#include "runir/kr/ps/ext/dl/parser/parsers.hpp"

#include <sstream>

namespace
{

template<typename Parser, typename Ast>
void parse_ast(const std::string& description,
               const Parser& parser,
               Ast& result,
               runir::kr::parser::ErrorHandlerType& error_handler,
               const char* failure_message)
{
    auto first = description.cbegin();
    if (!runir::kr::parser::parse_full(first, description.cend(), parser, result, error_handler))
        throw runir::kr::parser::DiagnosticContext::parse_error(error_handler, failure_message, first);
}

template<typename Parser, typename Ast>
Ast parse_ast(const std::string& description, const Parser& parser, const char* failure_message)
{
    auto result = Ast {};
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), errors);
    parse_ast(description, parser, result, error_handler, failure_message);
    return result;
}

}  // namespace

namespace runir::kr::ps::ext::dl::parser
{

void parse_module_ast(const std::string& description, ast::Module& result, runir::kr::parser::ErrorHandlerType& error_handler)
{
    parse_ast(description, module_root_parser(), result, error_handler, "Failed to parse DL module description.");
}

void parse_module_program_ast(const std::string& description, ast::ModuleProgram& result, runir::kr::parser::ErrorHandlerType& error_handler)
{
    parse_ast(description, module_program_root_parser(), result, error_handler, "Failed to parse DL module program description.");
}

ast::Module parse_module_ast(const std::string& description)
{
    return parse_ast<module_root_type, ast::Module>(description, module_root_parser(), "Failed to parse DL module description.");
}

ast::ModuleProgram parse_module_program_ast(const std::string& description)
{
    return parse_ast<module_program_root_type, ast::ModuleProgram>(description, module_program_root_parser(), "Failed to parse DL module program description.");
}

}  // namespace runir::kr::ps::ext::dl::parser
