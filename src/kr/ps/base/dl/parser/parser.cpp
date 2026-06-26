#include "runir/kr/ps/base/dl/parser/parser.hpp"

#include "runir/kr/ps/base/dl/parser/error_handler.hpp"
#include "runir/kr/ps/base/dl/parser/parsers.hpp"

#include <boost/spirit/home/x3.hpp>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string_view>


namespace
{

std::string strip_line_comments(std::string_view input)
{
    auto result = std::string(input);
    auto quoted = false;
    auto escaped = false;
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        const auto ch = result[i];
        if (quoted)
        {
            if (escaped)
            {
                escaped = false;
            }
            else if (ch == '\\')
            {
                escaped = true;
            }
            else if (ch == '"')
            {
                quoted = false;
            }
            continue;
        }
        if (ch == '"')
        {
            quoted = true;
            continue;
        }
        if (ch == ';')
        {
            while (i < result.size() && result[i] != '\n')
                result[i++] = ' ';
            if (i == result.size())
                break;
        }
    }
    return result;
}

}  // namespace

namespace runir::kr::ps::base::dl::parser
{

ast::Sketch<runir::kr::BaseFamilyTag> parse_sketch_ast(const std::string& description)
{
    namespace x3 = boost::spirit::x3;

    const auto stripped_description = strip_line_comments(description);
    auto first = stripped_description.cbegin();
    const auto last = stripped_description.cend();

    ast::Sketch<runir::kr::BaseFamilyTag> result;
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

}  // namespace runir::kr::ps::base::dl::parser
