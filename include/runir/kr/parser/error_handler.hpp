#ifndef RUNIR_KR_PARSER_ERROR_HANDLER_HPP_
#define RUNIR_KR_PARSER_ERROR_HANDLER_HPP_

#include "runir/kr/parser/config.hpp"

namespace runir::kr::parser
{

using ygg::diagnostics::format_error_at;
using ErrorHandlerBase = ygg::diagnostics::ErrorHandlerBase;

}  // namespace runir::kr::parser

#endif
