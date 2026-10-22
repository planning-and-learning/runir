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

identifier_type const identifier = "identifier";

boolean_feature_type const boolean_feature = "boolean_feature";
positive_literal_type const positive_literal = "positive_literal";
negative_literal_type const negative_literal = "negative_literal";
literal_type const literal = "literal";
conjunction_type const conjunction = "conjunction";
disjunction_type const disjunction = "disjunction";
classifier_type const classifier = "classifier";
classifier_root_type const classifier_root = "classifier_root";

const auto identifier_def = boost::spirit::x3::rule<IdentifierText, std::string> { "identifier_text" } =
    raw[lexeme[(alpha | char_('_')) >> *(alnum | char_('_') | char_('-'))]];

const auto symbol_value_def = identifier;
const auto symbol_section_def = context(":symbol")[(lit("(") >> keyword(":symbol")) > symbol_value_def > lit(")")];
const auto boolean_expression_section_def =
    context(":expression")[(lit("(") >> keyword(":expression")) > runir::kr::dl::grammar::parser::boolean_parser<runir::kr::ExtFamilyTag>() > lit(")")];

const auto boolean_feature_def =
    context("boolean feature")[(lit("(") >> lit(":") >> keyword("boolean")) > symbol_section_def > boolean_expression_section_def > lit(")")];

// DNF over feature symbols: literal := id | "(" "not" id ")"; conjunction := "(" "and" literal* ")";
// disjunction := "(" "or" conjunction* ")". The disjunction and conjunction are always written explicitly.
const auto positive_literal_def = identifier;
const auto negative_literal_def = context("negated literal")[(lit("(") >> keyword("not")) > identifier > lit(")")];
const auto literal_def = negative_literal | positive_literal;
const auto conjunction_def = context("conjunction")[(lit("(") >> keyword("and")) > *literal > lit(")")];
const auto disjunction_def = context("disjunction")[(lit("(") >> keyword("or")) > *conjunction > lit(")")];

const auto features_section_def = context(":features")[(lit("(") >> keyword(":features")) > *boolean_feature > lit(")")];
const auto expression_section_def = context(":expression")[(lit("(") >> keyword(":expression")) > disjunction > lit(")")];

const auto classifier_def =
    context("classifier")[(lit("(") >> lit(":") >> keyword("classifier")) > symbol_section_def > features_section_def > expression_section_def > lit(")")];
const auto classifier_root_def = context("classifier")[classifier > eoi];

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
