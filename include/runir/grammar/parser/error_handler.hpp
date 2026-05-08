#ifndef RUNIR_GRAMMAR_PARSER_ERROR_HANDLER_HPP_
#define RUNIR_GRAMMAR_PARSER_ERROR_HANDLER_HPP_

#include "runir/grammar/parser/declarations.hpp"

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>
#include <functional>
#include <string>
#include <unordered_map>

namespace runir::grammar::parser
{
namespace x3 = boost::spirit::x3;

using iterator_type = std::string::const_iterator;
using skipper_type = x3::ascii::space_type;
using phrase_context_type = x3::phrase_parse_context<skipper_type>::type;
using error_handler_type = x3::error_handler<iterator_type>;
using context_type = x3::context<x3::error_handler_tag, std::reference_wrapper<error_handler_type>, phrase_context_type>;

struct ErrorHandlerBase
{
    std::unordered_map<std::string, std::string> id_map;

    template<typename Iterator, typename Exception, typename Context>
    x3::error_handler_result on_error(Iterator& /*first*/, const Iterator& /*last*/, const Exception& x, const Context& context)
    {
        auto which = std::string(x.which());
        const auto it = id_map.find(which);
        if (it != id_map.end())
            which = it->second;

        auto& error_handler = x3::get<x3::error_handler_tag>(context).get();
        error_handler(x.where(), "Error! Expecting: " + which + " here:");

        return x3::error_handler_result::fail;
    }
};

}

#endif
