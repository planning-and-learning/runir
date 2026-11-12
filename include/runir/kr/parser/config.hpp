#ifndef RUNIR_KR_PARSER_CONFIG_HPP_
#define RUNIR_KR_PARSER_CONFIG_HPP_

#include "runir/kr/parser/declarations.hpp"

#include <boost/spirit/home/x3.hpp>
#include <functional>
#include <yggdrasil/diagnostics/x3.hpp>

namespace runir::kr::parser
{

using Context = ygg::diagnostics::ParserContext<ErrorHandlerType>;

}  // namespace runir::kr::parser

#endif
