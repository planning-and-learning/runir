#include "runir/kr/dl/grammar/parser/base/parser.hpp"

#include "runir/kr/dl/grammar/parser/base/grammar_parsers.hpp"
#include "runir/kr/dl/grammar/parser/error_handler.hpp"

#include <boost/spirit/home/x3.hpp>
#include <functional>
#include <sstream>
#include <stdexcept>

namespace runir::kr::dl::grammar::parser::base
{

runir::kr::dl::grammar::ast::Grammar<runir::kr::BaseFamilyTag> parse_grammar_ast(const std::string& description)
{
    namespace x3 = boost::spirit::x3;

    auto first = description.cbegin();
    const auto last = description.cend();

    runir::kr::dl::grammar::ast::Grammar<runir::kr::BaseFamilyTag> result;
    std::ostringstream errors;
    error_handler_type error_handler(first, last, errors);

    const auto grammar = x3::with<x3::error_handler_tag>(std::ref(error_handler))[runir::kr::dl::grammar::parser::base::grammar::grammar_root_parser()];
    const auto success = x3::phrase_parse(first, last, grammar, x3::ascii::space, result);

    if (!success || first != last)
    {
        auto message = errors.str();
        if (message.empty())
            message = "Failed to parse grammar description.";
        throw std::runtime_error(message);
    }

    return result;
}

}  // namespace runir::kr::dl::grammar::parser::base
