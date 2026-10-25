#ifndef RUNIR_KR_PARSER_SEXPRESSION_AST_HPP_
#define RUNIR_KR_PARSER_SEXPRESSION_AST_HPP_

#include "runir/kr/parser/ast.hpp"
#include "runir/kr/parser/ast_adapted.hpp"

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <vector>

/// Generic s-expressions, read by sections whose structure the semantic parser checks (the :datalog section).
namespace runir::kr::parser::ast
{
struct SList;

struct SExpression : boost::spirit::x3::variant<Identifier, boost::spirit::x3::forward_ast<SList>>, boost::spirit::x3::position_tagged
{
    using base_type::base_type;
    using base_type::operator=;
};

struct SList : boost::spirit::x3::position_tagged
{
    std::vector<SExpression> items;
};

/// (:datalog rule...): each rule an s-expression, interpreted by the semantic parser.
struct DatalogSection : boost::spirit::x3::position_tagged
{
    std::vector<SExpression> rules;
};
}  // namespace runir::kr::parser::ast

BOOST_FUSION_ADAPT_STRUCT(runir::kr::parser::ast::SList, items)
BOOST_FUSION_ADAPT_STRUCT(runir::kr::parser::ast::DatalogSection, rules)

#endif
