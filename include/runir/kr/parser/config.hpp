#ifndef RUNIR_KR_PARSER_CONFIG_HPP_
#define RUNIR_KR_PARSER_CONFIG_HPP_

#include "runir/kr/parser/declarations.hpp"

#include <boost/spirit/home/x3.hpp>
#include <functional>
#include <yggdrasil/diagnostics/x3.hpp>

namespace runir::kr::parser
{
namespace x3 = boost::spirit::x3;

using ErrorHandlerTag = ygg::diagnostics::ErrorHandlerTag;

inline auto keyword(const char* value)
{
    return x3::rule<class Keyword> { value } = x3::lexeme[x3::lit(value) >> !(x3::ascii::alnum | x3::char_('_') | x3::char_('-'))];
}

inline auto skipper() { return x3::ascii::space | (';' >> *(x3::char_ - x3::eol) >> (x3::eol | x3::eoi)); }

using PhraseContext = x3::phrase_parse_context<decltype(skipper())>::type;
using Context = x3::context<ErrorHandlerTag, std::reference_wrapper<ErrorHandlerType>, PhraseContext>;

}  // namespace runir::kr::parser

#endif
