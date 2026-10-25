#ifndef RUNIR_KR_PARSER_SEXPRESSION_HPP_
#define RUNIR_KR_PARSER_SEXPRESSION_HPP_

#include "runir/kr/parser/config.hpp"
#include "runir/kr/parser/error_handler.hpp"
#include "runir/kr/parser/sexpression_ast.hpp"

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <string>

/// Generic s-expressions, read by sections whose structure the semantic parser checks (the :datalog section).
namespace runir::kr::parser::grammar
{
struct SAtomClass : boost::spirit::x3::annotate_on_success
{
};
struct SAtomTextClass
{
};
struct SListClass : boost::spirit::x3::annotate_on_success
{
};
struct SExpressionClass : boost::spirit::x3::annotate_on_success
{
};
struct DatalogSectionClass : boost::spirit::x3::annotate_on_success
{
};

using boost::spirit::x3::lit;
using boost::spirit::x3::ascii::char_;
using boost::spirit::x3::ascii::space;

inline constexpr boost::spirit::x3::rule<SAtomClass, ast::Identifier> s_atom = "atom";
inline constexpr boost::spirit::x3::rule<SListClass, ast::SList> s_list = "list";
inline constexpr boost::spirit::x3::rule<SExpressionClass, ast::SExpression> s_expression = "s-expression";
inline constexpr boost::spirit::x3::rule<DatalogSectionClass, ast::DatalogSection> datalog_section = "datalog section";

inline constexpr auto s_atom_def = boost::spirit::x3::rule<SAtomTextClass, std::string> { "atom text" } =
    boost::spirit::x3::raw[boost::spirit::x3::lexeme[+(char_ - space - char_('(') - char_(')') - char_(';') - char_('"'))]];
inline constexpr auto s_list_def = (lit('(') > *s_expression) > lit(')');
inline constexpr auto s_expression_def = s_list | s_atom;
inline constexpr auto datalog_section_def = ((lit('(') >> keyword(":datalog")) > *s_expression) > lit(')');

BOOST_SPIRIT_DEFINE(s_atom, s_list, s_expression, datalog_section)
}  // namespace runir::kr::parser::grammar

#endif
