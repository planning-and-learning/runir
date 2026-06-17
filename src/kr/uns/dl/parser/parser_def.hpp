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
dnf_literal_type const dnf_literal = "dnf_literal";
dnf_clause_type const dnf_clause = "dnf_clause";
dnf_type const dnf = "dnf";
classifier_type const classifier = "classifier";
classifier_root_type const classifier_root = "classifier_root";

inline auto identifier_parser() { return raw[lexeme[(alpha | char_('_')) >> *(alnum | char_('_') | char_('-'))]]; }

inline auto quoted_string_parser() { return lexeme[lit('"') >> raw[*('\\' >> char_ | (char_ - '"'))] >> lit('"')]; }

const auto symbol_value_def = identifier_parser();
const auto symbol_section_def = lit("(") > lit(":symbol") > symbol_value_def > lit(")");
const auto description_section_def = lit("(") > lit(":description") > quoted_string_parser() > lit(")");
const auto boolean_expression_section_def = lit("(") > lit(":expression") > runir::kr::dl::grammar::parser::uns::grammar::boolean_parser() > lit(")");

const auto boolean_feature_def =
    (lit("(") >> lit(":") >> lit("boolean")) > symbol_section_def > description_section_def > boolean_expression_section_def > lit(")");

// DNF over feature symbols: literal := id | "(" "not" id ")"; clause := "(" "and" literal* ")";
// dnf := "(" "or" clause* ")". The disjunction and conjunction are always written explicitly.
const auto dnf_literal_def =
    ((lit("(") >> lit("not")) > identifier_parser() > lit(")") >> attr(true)) | (identifier_parser() >> attr(false));
const auto dnf_clause_def = (lit("(") >> lit("and")) > *dnf_literal > lit(")");
const auto dnf_def = (lit("(") >> lit("or")) > *dnf_clause > lit(")");

const auto features_section_def = lit("(") > lit(":features") > *boolean_feature > lit(")");
const auto expression_section_def = lit("(") > lit(":expression") > dnf > lit(")");

const auto classifier_def =
    (lit("(") >> lit(":") >> lit("classifier")) > symbol_section_def > description_section_def > features_section_def > expression_section_def > lit(")");
const auto classifier_root_def = classifier > eoi;

BOOST_SPIRIT_DEFINE(boolean_feature, dnf_literal, dnf_clause, dnf, classifier, classifier_root)

struct BooleanFeatureClass : x3::annotate_on_success
{
};
struct DnfLiteralClass : x3::annotate_on_success
{
};
struct DnfClauseClass : x3::annotate_on_success
{
};
struct DnfClass : x3::annotate_on_success
{
};
struct ClassifierClass : x3::annotate_on_success
{
};
struct ClassifierRootClass : ErrorHandlerBase
{
};

boolean_feature_type const& boolean_feature_parser() { return boolean_feature; }
dnf_literal_type const& dnf_literal_parser() { return dnf_literal; }
dnf_clause_type const& dnf_clause_parser() { return dnf_clause; }
dnf_type const& dnf_parser() { return dnf; }
classifier_type const& classifier_parser() { return classifier; }
classifier_root_type const& classifier_root_parser() { return classifier_root; }

}  // namespace runir::kr::uns::dl::parser

#endif
