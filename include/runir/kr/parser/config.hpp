#ifndef RUNIR_KR_PARSER_CONFIG_HPP_
#define RUNIR_KR_PARSER_CONFIG_HPP_

#include "runir/kr/parser/declarations.hpp"

#include <boost/spirit/home/x3.hpp>
#include <functional>
#include <yggdrasil/diagnostics/x3.hpp>

namespace runir::kr::parser
{

using ErrorHandlerTag = ygg::diagnostics::ErrorHandlerTag;

inline auto keyword(const char* value)
{
    return boost::spirit::x3::rule<class Keyword> { value } =
               boost::spirit::x3::lexeme[boost::spirit::x3::lit(value)
                                         >> !(boost::spirit::x3::ascii::alnum | boost::spirit::x3::char_('_') | boost::spirit::x3::char_('-'))];
}

using Context = ygg::diagnostics::ParserContext<ErrorHandlerType>;

}  // namespace runir::kr::parser

#endif
