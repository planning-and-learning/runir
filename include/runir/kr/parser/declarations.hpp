#ifndef RUNIR_KR_PARSER_DECLARATIONS_HPP_
#define RUNIR_KR_PARSER_DECLARATIONS_HPP_

#include <string>

namespace runir::kr::parser
{

using Iterator = std::string::const_iterator;

template<typename It>
class ErrorHandler;

using ErrorHandlerType = ErrorHandler<Iterator>;

}  // namespace runir::kr::parser

#endif
