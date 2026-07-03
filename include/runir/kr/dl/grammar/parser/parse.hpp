#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_PARSE_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_PARSE_HPP_

#include "runir/kr/dl/grammar/parser/error_handler.hpp"

#include <boost/spirit/home/x3.hpp>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>

namespace runir::kr::dl::grammar::parser
{

template<typename Ast, typename Parser>
Ast parse_ast(const std::string& description, const Parser& parser, const char* fallback_message)
{
    namespace x3 = boost::spirit::x3;
    auto first = description.cbegin();
    const auto last = description.cend();
    Ast result;
    std::ostringstream errors;
    error_handler_type error_handler(first, last, errors);
    const auto grammar = x3::with<x3::error_handler_tag>(std::ref(error_handler))[parser];
    const auto success = x3::phrase_parse(first, last, grammar, x3::ascii::space, result);
    if (!success || first != last)
    {
        auto message = errors.str();
        if (message.empty())
            message = fallback_message;
        throw std::runtime_error(message);
    }
    return result;
}

}  // namespace runir::kr::dl::grammar::parser

#endif
