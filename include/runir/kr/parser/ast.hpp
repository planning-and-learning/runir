#ifndef RUNIR_KR_PARSER_AST_HPP_
#define RUNIR_KR_PARSER_AST_HPP_

#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <string>

namespace runir::kr::parser::ast
{

struct Identifier : boost::spirit::x3::position_tagged
{
    std::string text;
};

}  // namespace runir::kr::parser::ast

#endif
