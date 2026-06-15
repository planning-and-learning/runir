#include "runir/kr/ps/uns/dl/parser/parser.hpp"

#include "runir/kr/ps/uns/dl/parser/error_handler.hpp"
#include "runir/kr/ps/uns/dl/parser/parsers.hpp"

#include <boost/spirit/home/x3.hpp>
#include <functional>
#include <sstream>
#include <stdexcept>

namespace runir::kr::ps::uns::dl::parser
{

ast::Sketch<runir::kr::UnsFamilyTag> parse_sketch_ast(const std::string& description)
{
    namespace x3 = boost::spirit::x3;

    auto first = description.cbegin();
    const auto last = description.cend();

    ast::Sketch<runir::kr::UnsFamilyTag> result;
    std::ostringstream errors;
    error_handler_type error_handler(first, last, errors);

    const auto parser = x3::with<x3::error_handler_tag>(std::ref(error_handler))[sketch_root_parser()];
    const auto success = x3::phrase_parse(first, last, parser, x3::ascii::space, result);

    if (!success || first != last)
    {
        auto message = errors.str();
        if (message.empty())
            message = "Failed to parse DL general sketch description.";
        throw std::runtime_error(message);
    }

    return result;
}

}  // namespace runir::kr::ps::uns::dl::parser
