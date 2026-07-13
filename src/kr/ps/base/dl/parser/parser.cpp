#include "runir/kr/ps/base/dl/parser/parser.hpp"

#include "runir/kr/parser/diagnostics.hpp"
#include "runir/kr/parser/parser.hpp"
#include "runir/kr/ps/base/dl/parser/parsers.hpp"

#include <sstream>

namespace runir::kr::ps::base::dl::parser
{

ast::Sketch<runir::kr::BaseFamilyTag> parse_sketch_ast(const std::string& description)
{
    auto first = description.cbegin();
    const auto last = description.cend();
    auto result = ast::Sketch<runir::kr::BaseFamilyTag> {};
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::parser::ErrorHandlerType(first, last, errors);

    if (!runir::kr::parser::parse_full(first, last, sketch_root_parser(), result, error_handler))
        throw runir::kr::parser::DiagnosticContext::parse_error(error_handler, "Failed to parse DL general sketch description.", first);

    return result;
}

}  // namespace runir::kr::ps::base::dl::parser
