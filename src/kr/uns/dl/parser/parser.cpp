#include "runir/kr/uns/dl/parser/parser.hpp"

#include "runir/kr/parser/diagnostics.hpp"
#include "runir/kr/parser/parser.hpp"
#include "runir/kr/uns/dl/parser/parsers.hpp"

#include <sstream>

namespace runir::kr::uns::dl::parser
{

ast::Classifier parse_classifier_ast(const std::string& description)
{
    auto first = description.cbegin();
    const auto last = description.cend();
    auto result = ast::Classifier {};
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(first, last, errors);

    if (!runir::kr::parser::parse_full(first, last, classifier_root_parser(), result, error_handler))
        throw runir::kr::parser::DiagnosticContext::parse_error(error_handler, "Failed to parse DL classifier description.", first);

    return result;
}

}  // namespace runir::kr::uns::dl::parser
