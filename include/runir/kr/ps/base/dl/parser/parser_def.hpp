#ifndef RUNIR_KR_PS_BASE_DL_PARSER_PARSER_DEF_HPP_
#define RUNIR_KR_PS_BASE_DL_PARSER_PARSER_DEF_HPP_

#include "runir/kr/dl/grammar/parser/parsers.hpp"
#include "runir/kr/parser/error_handler.hpp"
#include "runir/kr/ps/base/dl/ast/ast.hpp"
#include "runir/kr/ps/base/dl/ast/ast_adapted.hpp"
#include "runir/kr/ps/base/dl/parser/parsers.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <string>

namespace runir::kr::ps::base::dl::parser
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

struct IdentifierText
{
};

identifier_type const identifier = "identifier";

base_boolean_feature_type const boolean_feature = "boolean_feature";
base_numerical_feature_type const numerical_feature = "numerical_feature";
base_feature_type const feature = "feature";

positive_condition_type const positive_condition = "positive_condition";
negative_condition_type const negative_condition = "negative_condition";
equal_zero_condition_type const equal_zero_condition = "equal_zero_condition";
greater_zero_condition_type const greater_zero_condition = "greater_zero_condition";
base_condition_observation_type const condition_observation = "condition_observation";
base_condition_type const condition = "condition";

positive_effect_type const positive_effect = "positive_effect";
negative_effect_type const negative_effect = "negative_effect";
unchanged_effect_type const unchanged_effect = "unchanged_effect";
increases_effect_type const increases_effect = "increases_effect";
decreases_effect_type const decreases_effect = "decreases_effect";
base_effect_observation_type const effect_observation = "effect_observation";
base_effect_type const effect = "effect";

base_rule_type const rule = "rule";
base_sketch_type const sketch = "sketch";
base_sketch_root_type const sketch_root = "sketch_root";

const auto identifier_def = x3::rule<IdentifierText, std::string> { "identifier_text" } =
    raw[lexeme[(alpha | char_('_')) >> *(alnum | char_('_') | char_('-'))]];

const auto symbol_value_def = identifier;
const auto symbol_section_def = lit("(") > lit(":symbol") > symbol_value_def > lit(")");
const auto boolean_expression_section_def = lit("(") > lit(":expression") > runir::kr::dl::grammar::parser::boolean_parser<runir::kr::BaseFamilyTag>()
                                            > lit(")");
const auto numerical_expression_section_def = lit("(") > lit(":expression") > runir::kr::dl::grammar::parser::numerical_parser<runir::kr::BaseFamilyTag>()
                                              > lit(")");

const auto boolean_feature_def = (lit("(") >> lit(":") >> lit(runir::kr::ps::dl::BooleanFeature::keyword)) > symbol_section_def > boolean_expression_section_def
                                 > lit(")");
const auto numerical_feature_def = (lit("(") >> lit(":") >> lit(runir::kr::ps::dl::NumericalFeature::keyword)) > symbol_section_def
                                   > numerical_expression_section_def > lit(")");
const auto feature_def = boolean_feature | numerical_feature;

const auto positive_condition_def = lit(ast::Positive::keyword) >> attr(ast::Positive {});
const auto negative_condition_def = lit(ast::Negative::keyword) >> attr(ast::Negative {});
const auto equal_zero_condition_def = lit(ast::EqualZero::keyword) >> attr(ast::EqualZero {});
const auto greater_zero_condition_def = lit(ast::GreaterZero::keyword) >> attr(ast::GreaterZero {});
const auto condition_observation_def = positive_condition | negative_condition | equal_zero_condition | greater_zero_condition;
const auto condition_def = (lit("(") >> condition_observation) > identifier > lit(")");

const auto positive_effect_def = lit(ast::Positive::keyword) >> attr(ast::Positive {});
const auto negative_effect_def = lit(ast::Negative::keyword) >> attr(ast::Negative {});
const auto unchanged_effect_def = lit(ast::Unchanged::keyword) >> attr(ast::Unchanged {});
const auto increases_effect_def = lit(ast::Increases::keyword) >> attr(ast::Increases {});
const auto decreases_effect_def = lit(ast::Decreases::keyword) >> attr(ast::Decreases {});
const auto effect_observation_def = positive_effect | negative_effect | unchanged_effect | increases_effect | decreases_effect;
const auto effect_def = (lit("(") >> effect_observation) > identifier > lit(")");

const auto conditions_section_def = lit("(") > lit(":conditions") > *condition > lit(")");
const auto effects_section_def = lit("(") > lit(":effects") > *effect > lit(")");
const auto rule_expression_section_def = lit("(") > lit(":expression") > conditions_section_def > effects_section_def > lit(")");
const auto features_section_def = lit("(") > lit(":features") > *feature > lit(")");
const auto rules_section_def = lit("(") > lit(":rules") > *rule > lit(")");

const auto rule_def = (lit("(") >> lit(":rule")) > symbol_section_def > rule_expression_section_def > lit(")");
const auto sketch_def = (lit("(") >> lit(":sketch")) > features_section_def > rules_section_def > lit(")");
const auto sketch_root_def = sketch > eoi;

BOOST_SPIRIT_DEFINE(identifier, boolean_feature, numerical_feature, feature)
BOOST_SPIRIT_DEFINE(positive_condition, negative_condition, equal_zero_condition, greater_zero_condition, condition_observation, condition)
BOOST_SPIRIT_DEFINE(positive_effect, negative_effect, unchanged_effect, increases_effect, decreases_effect, effect_observation, effect)
BOOST_SPIRIT_DEFINE(rule, sketch, sketch_root)

struct IdentifierClass : runir::kr::parser::ErrorHandlerBase
{
};

struct BooleanFeatureClass : x3::annotate_on_success
{
};
struct NumericalFeatureClass : x3::annotate_on_success
{
};
struct FeatureClass : x3::annotate_on_success
{
};

struct PositiveConditionClass : x3::annotate_on_success
{
};
struct NegativeConditionClass : x3::annotate_on_success
{
};
struct EqualZeroConditionClass : x3::annotate_on_success
{
};
struct GreaterZeroConditionClass : x3::annotate_on_success
{
};
struct ConditionObservationClass : x3::annotate_on_success
{
};
struct ConditionClass : x3::annotate_on_success
{
};

struct PositiveEffectClass : x3::annotate_on_success
{
};
struct NegativeEffectClass : x3::annotate_on_success
{
};
struct UnchangedEffectClass : x3::annotate_on_success
{
};
struct IncreasesEffectClass : x3::annotate_on_success
{
};
struct DecreasesEffectClass : x3::annotate_on_success
{
};
struct EffectObservationClass : x3::annotate_on_success
{
};
struct EffectClass : x3::annotate_on_success
{
};

struct RuleClass : x3::annotate_on_success
{
};
struct SketchClass : x3::annotate_on_success
{
};
struct SketchRootClass : runir::kr::parser::ErrorHandlerBase
{
};

}  // namespace runir::kr::ps::base::dl::parser

#endif
