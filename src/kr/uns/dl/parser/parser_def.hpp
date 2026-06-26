#ifndef RUNIR_SRC_KR_UNS_DL_PARSER_PARSER_DEF_HPP_
#define RUNIR_SRC_KR_UNS_DL_PARSER_PARSER_DEF_HPP_

#include "runir/kr/dl/grammar/parser/uns/grammar_parsers.hpp"
#include "runir/kr/uns/dl/ast/ast.hpp"
#include "runir/kr/uns/dl/ast/ast_adapted.hpp"
#include "runir/kr/uns/dl/parser/error_handler.hpp"
#include "runir/kr/uns/dl/parser/parsers.hpp"

#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>

namespace runir::kr::uns::dl::parser
{
namespace x3 = boost::spirit::x3;

using x3::attr;
using x3::eoi;
using x3::lexeme;
using x3::lit;
using x3::raw;

using x3::ascii::alnum;
using x3::ascii::alpha;
using x3::ascii::char_;

boolean_feature_type const boolean_feature = "boolean_feature";
positive_literal_type const positive_literal = "positive_literal";
negative_literal_type const negative_literal = "negative_literal";
literal_type const literal = "literal";
conjunction_type const conjunction = "conjunction";
disjunction_type const disjunction = "disjunction";
classifier_type const classifier = "classifier";
classifier_root_type const classifier_root = "classifier_root";

inline auto identifier_parser() { return raw[lexeme[(alpha | char_('_')) >> *(alnum | char_('_') | char_('-'))]]; }

inline auto quoted_string_parser() { return lexeme[lit('"') >> raw[*('\\' >> char_ | (char_ - '"'))] >> lit('"')]; }

const auto symbol_value_def = identifier_parser();
const auto symbol_section_def = lit("(") > lit(":symbol") > symbol_value_def > lit(")");
const auto boolean_expression_section_def = lit("(") > lit(":expression") > runir::kr::dl::grammar::parser::uns::grammar::boolean_parser() > lit(")");

const auto boolean_feature_def = (lit("(") >> lit(":") >> lit("boolean")) > symbol_section_def > boolean_expression_section_def > lit(")");

// DNF over feature symbols: literal := id | "(" "not" id ")"; conjunction := "(" "and" literal* ")";
// disjunction := "(" "or" conjunction* ")". The disjunction and conjunction are always written explicitly.
const auto positive_literal_def = identifier_parser();
const auto negative_literal_def = (lit("(") >> lit("not")) > identifier_parser() > lit(")");
const auto literal_def = negative_literal | positive_literal;
const auto conjunction_def = (lit("(") >> lit("and")) > *literal > lit(")");
const auto disjunction_def = (lit("(") >> lit("or")) > *conjunction > lit(")");

const auto features_section_def = lit("(") > lit(":features") > *boolean_feature > lit(")");
const auto expression_section_def = lit("(") > lit(":expression") > disjunction > lit(")");

const auto classifier_def = (lit("(") >> lit(":") >> lit("classifier")) > symbol_section_def > features_section_def > expression_section_def > lit(")");
const auto classifier_root_def = classifier > eoi;

BOOST_SPIRIT_DEFINE(boolean_feature, positive_literal, negative_literal, literal, conjunction, disjunction, classifier, classifier_root)

struct BooleanFeatureClass : x3::annotate_on_success
{
};
struct PositiveLiteralClass : x3::annotate_on_success
{
};
struct NegativeLiteralClass : x3::annotate_on_success
{
};
struct LiteralClass : x3::annotate_on_success
{
};
struct ConjunctionClass : x3::annotate_on_success
{
};
struct DisjunctionClass : x3::annotate_on_success
{
};
struct ClassifierClass : x3::annotate_on_success
{
};
struct ClassifierRootClass : ErrorHandlerBase
{
};

boolean_feature_type const& boolean_feature_parser() { return boolean_feature; }
positive_literal_type const& positive_literal_parser() { return positive_literal; }
negative_literal_type const& negative_literal_parser() { return negative_literal; }
literal_type const& literal_parser() { return literal; }
conjunction_type const& conjunction_parser() { return conjunction; }
disjunction_type const& disjunction_parser() { return disjunction; }
classifier_type const& classifier_parser() { return classifier; }
classifier_root_type const& classifier_root_parser() { return classifier_root; }

}  // namespace runir::kr::uns::dl::parser

#endif
