#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_PARSE_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_PARSE_HPP_

#include "runir/kr/parser/diagnostics.hpp"
#include "runir/kr/parser/parser.hpp"

#include <boost/spirit/home/x3/support/traits/is_parser.hpp>
#include <concepts>
#include <sstream>
#include <string>

namespace runir::kr::dl::grammar::parser
{

template<std::default_initializable Ast, typename Parser>
    requires boost::spirit::x3::traits::is_parser<Parser>::value
void parse_ast(const std::string& description,
               const Parser& parser,
               Ast& result,
               runir::kr::parser::ErrorHandlerType& error_handler,
               const char* fallback_message)
{
    auto first = description.cbegin();
    if (!runir::kr::parser::parse_full(first, description.cend(), parser, result, error_handler))
        throw runir::kr::parser::DiagnosticContext::parse_error(error_handler, fallback_message, first);
}

template<std::default_initializable Ast, typename Parser>
    requires boost::spirit::x3::traits::is_parser<Parser>::value
Ast parse_ast(const std::string& description, const Parser& parser, const char* fallback_message)
{
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), errors);
    auto result = Ast {};
    parse_ast(description, parser, result, error_handler, fallback_message);
    return result;
}

}  // namespace runir::kr::dl::grammar::parser

#endif
