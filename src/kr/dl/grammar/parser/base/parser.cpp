#include "runir/kr/dl/grammar/parser/base/parser.hpp"

#include "runir/kr/dl/grammar/parser/base/parsers.hpp"
#include "runir/kr/dl/grammar/parser/error_handler.hpp"

#include <boost/spirit/home/x3.hpp>
#include <functional>
#include <sstream>
#include <stdexcept>

namespace runir::kr::dl::grammar::parser::base
{

namespace
{
template<typename Ast, typename Parser>
Ast parse_ast(const std::string& description, const Parser& parser)
{
    namespace x3 = boost::spirit::x3;
    auto first = description.cbegin();
    const auto last = description.cend();
    Ast result;
    std::ostringstream errors;
    error_handler_type error_handler(first, last, errors);
    const auto grammar = x3::with<x3::error_handler_tag>(std::ref(error_handler))[parser];
    const auto success = x3::phrase_parse(first, last, grammar, x3::ascii::space, result);
    if (!success || first != last)
    {
        auto message = errors.str();
        if (message.empty())
            message = "Failed to parse DL constructor description.";
        throw std::runtime_error(message);
    }
    return result;
}
}  // namespace

template<>
ast::ConceptConstructor<runir::kr::dl::BaseFamilyTag> parse_concept_ast<runir::kr::dl::BaseFamilyTag>(const std::string& description)
{
    return parse_ast<ast::ConceptConstructor<runir::kr::dl::BaseFamilyTag>>(description, base_concept_root_parser());
}

template<>
ast::RoleConstructor<runir::kr::dl::BaseFamilyTag> parse_role_ast<runir::kr::dl::BaseFamilyTag>(const std::string& description)
{
    return parse_ast<ast::RoleConstructor<runir::kr::dl::BaseFamilyTag>>(description, base_role_root_parser());
}

}  // namespace runir::kr::dl::grammar::parser::base
