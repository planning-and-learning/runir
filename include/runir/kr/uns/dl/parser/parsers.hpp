#ifndef RUNIR_KR_UNS_DL_PARSER_PARSERS_HPP_
#define RUNIR_KR_UNS_DL_PARSER_PARSERS_HPP_

#include "runir/kr/uns/dl/ast/ast.hpp"

#include <boost/spirit/home/x3.hpp>

namespace runir::kr::uns::dl::parser
{
namespace x3 = boost::spirit::x3;

struct BooleanFeatureClass;
struct PositiveLiteralClass;
struct NegativeLiteralClass;
struct LiteralClass;
struct ConjunctionClass;
struct DisjunctionClass;
struct ClassifierClass;
struct ClassifierRootClass;

using boolean_feature_type = x3::rule<BooleanFeatureClass, ast::BooleanFeature>;
using positive_literal_type = x3::rule<PositiveLiteralClass, ast::PositiveLiteral>;
using negative_literal_type = x3::rule<NegativeLiteralClass, ast::NegativeLiteral>;
using literal_type = x3::rule<LiteralClass, ast::Literal>;
using conjunction_type = x3::rule<ConjunctionClass, ast::Conjunction>;
using disjunction_type = x3::rule<DisjunctionClass, ast::Disjunction>;
using classifier_type = x3::rule<ClassifierClass, ast::Classifier>;
using classifier_root_type = x3::rule<ClassifierRootClass, ast::Classifier>;

BOOST_SPIRIT_DECLARE(boolean_feature_type,
                     positive_literal_type,
                     negative_literal_type,
                     literal_type,
                     conjunction_type,
                     disjunction_type,
                     classifier_type,
                     classifier_root_type)

boolean_feature_type const& boolean_feature_parser();
positive_literal_type const& positive_literal_parser();
negative_literal_type const& negative_literal_parser();
literal_type const& literal_parser();
conjunction_type const& conjunction_parser();
disjunction_type const& disjunction_parser();
classifier_type const& classifier_parser();
classifier_root_type const& classifier_root_parser();

}  // namespace runir::kr::uns::dl::parser

#endif
