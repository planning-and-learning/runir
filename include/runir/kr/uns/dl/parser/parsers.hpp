#ifndef RUNIR_KR_UNS_DL_PARSER_PARSERS_HPP_
#define RUNIR_KR_UNS_DL_PARSER_PARSERS_HPP_

#include "runir/kr/uns/dl/ast/ast.hpp"

#include <boost/spirit/home/x3.hpp>
#include <vector>

namespace runir::kr::uns::dl::parser
{
namespace x3 = boost::spirit::x3;

struct BooleanFeatureClass;
struct DnfLiteralClass;
struct DnfClauseClass;
struct DnfClass;
struct ClassifierClass;
struct ClassifierRootClass;

using boolean_feature_type = x3::rule<BooleanFeatureClass, ast::BooleanFeature>;
using dnf_literal_type = x3::rule<DnfLiteralClass, ast::DnfLiteral>;
using dnf_clause_type = x3::rule<DnfClauseClass, std::vector<ast::DnfLiteral>>;
using dnf_type = x3::rule<DnfClass, std::vector<std::vector<ast::DnfLiteral>>>;
using classifier_type = x3::rule<ClassifierClass, ast::Classifier>;
using classifier_root_type = x3::rule<ClassifierRootClass, ast::Classifier>;

BOOST_SPIRIT_DECLARE(boolean_feature_type, dnf_literal_type, dnf_clause_type, dnf_type, classifier_type, classifier_root_type)

boolean_feature_type const& boolean_feature_parser();
dnf_literal_type const& dnf_literal_parser();
dnf_clause_type const& dnf_clause_parser();
dnf_type const& dnf_parser();
classifier_type const& classifier_parser();
classifier_root_type const& classifier_root_parser();

}  // namespace runir::kr::uns::dl::parser

#endif
