#include "runir/kr/ps/ext/dl/parser/parser.hpp"

#include "runir/kr/ps/ext/dl/parser/error_handler.hpp"
#include "runir/kr/ps/ext/dl/parser/parsers.hpp"

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

template<typename Parser, typename Ast>
Ast parse_ast(const std::string& description, const Parser& parser, const char* failure_message)
{
    namespace x3 = boost::spirit::x3;

    const auto stripped_description = strip_line_comments(description);
    auto first = stripped_description.cbegin();
    const auto last = stripped_description.cend();

    auto result = Ast {};
    auto errors = std::ostringstream {};
    auto error_handler = runir::kr::ps::ext::dl::parser::error_handler_type(first, last, errors);
    const auto wrapped = x3::with<x3::error_handler_tag>(std::ref(error_handler))[parser];
    const auto success = x3::phrase_parse(first, last, wrapped, x3::ascii::space, result);

    if (!success || first != last)
    {
        auto message = errors.str();
        if (message.empty())
            message = failure_message;
        throw std::runtime_error(message);
    }

    return result;
}

}  // namespace

namespace runir::kr::ps::ext::dl::parser
{

ast::Module parse_module_ast(const std::string& description)
{
    return parse_ast<module_root_type, ast::Module>(description, module_root_parser(), "Failed to parse DL module description.");
}

ast::ModuleProgram parse_module_program_ast(const std::string& description)
{
    return parse_ast<module_program_root_type, ast::ModuleProgram>(description, module_program_root_parser(), "Failed to parse DL module program description.");
}

}  // namespace runir::kr::ps::ext::dl::parser
