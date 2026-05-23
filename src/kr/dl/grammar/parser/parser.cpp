#include "runir/kr/dl/grammar/parser/parser.hpp"

#include "runir/kr/dl/grammar/parser/error_handler.hpp"
#include "runir/kr/dl/grammar/parser/parsers.hpp"

#include <boost/spirit/home/x3.hpp>
#include <functional>
#include <sstream>
#include <stdexcept>

namespace runir::kr::dl::grammar::parser
{

template<>
ast::Grammar<runir::kr::dl::BaseFamilyTag> parse_grammar_ast<runir::kr::dl::BaseFamilyTag>(const std::string& description)
{
    namespace x3 = boost::spirit::x3;

    auto first = description.cbegin();
    const auto last = description.cend();

    ast::Grammar<runir::kr::dl::BaseFamilyTag> result;
    std::ostringstream errors;
    error_handler_type error_handler(first, last, errors);

    const auto grammar = x3::with<x3::error_handler_tag>(std::ref(error_handler))[grammar_root_parser()];
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

}
