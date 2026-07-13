#ifndef RUNIR_KR_PARSER_ERROR_HANDLER_HPP_
#define RUNIR_KR_PARSER_ERROR_HANDLER_HPP_

#include "runir/kr/parser/config.hpp"

#include <sstream>
#include <string>

namespace runir::kr::parser
{

template<typename It>
std::string format_error_at(const ErrorHandler<It>& source, It position, const std::string& message)
{
    auto output = std::ostringstream {};
    auto formatter = ErrorHandler<It>(source.get_position_cache().first(), source.get_position_cache().last(), output, source.tabs());
    formatter(position, message);
    return output.str();
}

template<typename It>
std::string format_error_at(const ErrorHandler<It>& source, const x3::position_tagged& node, const std::string& message)
{
    if (node.id_first < 0 || node.id_last < 0)
        return message;
    return format_error_at(source, source.position_of(node).begin(), message);
}

struct ErrorHandlerBase
{
    template<typename It, typename Ast, typename ParserContext>
    void on_success(const It& first, const It& last, Ast& ast, const ParserContext& context)
    {
        auto& error_handler = x3::get<ErrorHandlerTag>(context).get();
        error_handler.tag(ast, first, last);
    }

    template<typename It, typename Exception, typename ParserContext>
    x3::error_handler_result on_error(It&, const It&, const Exception& error, const ParserContext& context)
    {
        auto message = "Error! Expecting: " + std::string(error.which()) + " here:";
        auto& error_handler = x3::get<ErrorHandlerTag>(context).get();
        error_handler.record_error(error.where(), message);
        error_handler(error.where(), message);
        return x3::error_handler_result::fail;
    }
};

}  // namespace runir::kr::parser

#endif
