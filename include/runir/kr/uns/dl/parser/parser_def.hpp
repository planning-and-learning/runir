#ifndef RUNIR_KR_UNS_DL_PARSER_PARSER_DEF_HPP_
#define RUNIR_KR_UNS_DL_PARSER_PARSER_DEF_HPP_

#include "runir/kr/dl/grammar/parser/parsers.hpp"
#include "runir/kr/parser/error_handler.hpp"
#include "runir/kr/uns/dl/ast/ast.hpp"
#include "runir/kr/uns/dl/ast/ast_adapted.hpp"
#include "runir/kr/uns/dl/parser/parsers.hpp"

#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <string>

namespace runir::kr::uns::dl::parser
{

using boost::spirit::x3::attr;
using boost::spirit::x3::eoi;
using boost::spirit::x3::lexeme;
using boost::spirit::x3::lit;
using boost::spirit::x3::raw;
using runir::kr::parser::keyword;
using ygg::diagnostics::context;

using boost::spirit::x3::ascii::alnum;
using boost::spirit::x3::ascii::alpha;
using boost::spirit::x3::ascii::char_;

struct IdentifierText
{
};

constexpr identifier_type identifier = "identifier";

constexpr boolean_feature_type boolean_feature = "boolean_feature";
constexpr positive_literal_type positive_literal = "positive_literal";
constexpr negative_literal_type negative_literal = "negative_literal";
constexpr literal_type literal = "literal";
constexpr conjunction_type conjunction = "conjunction";
constexpr disjunction_type disjunction = "disjunction";
constexpr classifier_type classifier = "classifier";
constexpr classifier_root_type classifier_root = "classifier_root";

constexpr auto identifier_def = boost::spirit::x3::rule<IdentifierText, std::string> { "identifier_text" } =
    raw[lexeme[(alpha | char_('_')) >> *(alnum | char_('_') | char_('-'))]];

constexpr auto symbol_value_def = identifier;
constexpr auto symbol_section_def = context(":symbol")[(lit("(") >> keyword(":symbol")) > symbol_value_def > lit(")")];
constexpr auto boolean_expression_section_def =
    context(":expression")[(lit("(") >> keyword(":expression")) > runir::kr::dl::grammar::parser::dl_boolean > lit(")")];

constexpr auto boolean_feature_def =
    context("boolean feature")[(lit("(") >> lit(":") >> keyword("boolean")) > symbol_section_def > boolean_expression_section_def > lit(")")];

// DNF over feature symbols: literal := id | "(" "not" id ")"; conjunction := "(" "and" literal* ")";
// disjunction := "(" "or" conjunction* ")". The disjunction and conjunction are always written explicitly.
constexpr auto positive_literal_def = identifier;
constexpr auto negative_literal_def = context("negated literal")[(lit("(") >> keyword("not")) > identifier > lit(")")];
constexpr auto literal_def = negative_literal | positive_literal;
constexpr auto conjunction_def = context("conjunction")[(lit("(") >> keyword("and")) > *literal > lit(")")];
constexpr auto disjunction_def = context("disjunction")[(lit("(") >> keyword("or")) > *conjunction > lit(")")];

constexpr auto features_section_def = context(":features")[(lit("(") >> keyword(":features")) > *boolean_feature > lit(")")];
constexpr auto expression_section_def = context(":expression")[(lit("(") >> keyword(":expression")) > disjunction > lit(")")];

constexpr auto classifier_def =
    context("classifier")[(lit("(") >> lit(":") >> keyword("classifier")) > symbol_section_def > features_section_def > expression_section_def > lit(")")];
constexpr auto classifier_root_def = context("classifier")[classifier > eoi];

BOOST_SPIRIT_DEFINE(identifier, boolean_feature, positive_literal, negative_literal, literal, conjunction, disjunction, classifier, classifier_root)

struct IdentifierClass : runir::kr::parser::ErrorHandlerBase
{
};

struct BooleanFeatureClass : boost::spirit::x3::annotate_on_success
{
};
struct PositiveLiteralClass : boost::spirit::x3::annotate_on_success
{
};
struct NegativeLiteralClass : boost::spirit::x3::annotate_on_success
{
};
struct LiteralClass : boost::spirit::x3::annotate_on_success
{
};
struct ConjunctionClass : boost::spirit::x3::annotate_on_success
{
};
struct DisjunctionClass : boost::spirit::x3::annotate_on_success
{
};
struct ClassifierClass : boost::spirit::x3::annotate_on_success
{
};
struct ClassifierRootClass : runir::kr::parser::ErrorHandlerBase
{
};

}  // namespace runir::kr::uns::dl::parser

#endif
