#ifndef RUNIR_KR_PARSER_PARSER_HPP_
#define RUNIR_KR_PARSER_PARSER_HPP_

#include "runir/kr/parser/config.hpp"

#include <boost/spirit/home/x3.hpp>
#include <functional>

namespace runir::kr::parser
{

template<typename Parser, typename Ast>
bool parse_full(Iterator& first, Iterator last, const Parser& parser, Ast& ast, ErrorHandlerType& error_handler)
{
    error_handler.clear_error();
    ast = Ast {};
    const auto grammar = x3::with<ErrorHandlerTag>(std::ref(error_handler))[parser];
    const auto success = x3::phrase_parse(first, last, grammar, skipper(), ast);
    if (success && first != last)
        error_handler.record_error(first, "Error! Unexpected trailing input here:");
    return success && first == last;
}

}  // namespace runir::kr::parser

#endif
