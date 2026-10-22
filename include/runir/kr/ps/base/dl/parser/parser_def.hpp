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

const auto identifier_def = boost::spirit::x3::rule<IdentifierText, std::string> { "identifier_text" } =
    raw[lexeme[(alpha | char_('_')) >> *(alnum | char_('_') | char_('-'))]];

const auto symbol_value_def = identifier;
const auto symbol_section_def = context(":symbol")[(lit("(") >> keyword(":symbol")) > symbol_value_def > lit(")")];
const auto boolean_expression_section_def =
    context(":expression")[(lit("(") >> keyword(":expression")) > runir::kr::dl::grammar::parser::boolean_parser<>() > lit(")")];
const auto numerical_expression_section_def =
    context(":expression")[(lit("(") >> keyword(":expression")) > runir::kr::dl::grammar::parser::numerical_parser<>() > lit(")")];

const auto boolean_feature_def = context("boolean feature")[(lit("(") >> lit(":") >> keyword(runir::kr::dl::BooleanTag::name)) > symbol_section_def
                                                            > boolean_expression_section_def > lit(")")];
const auto numerical_feature_def = context("numerical feature")[(lit("(") >> lit(":") >> keyword(runir::kr::dl::NumericalTag::name))
                                                                > symbol_section_def > numerical_expression_section_def > lit(")")];
const auto feature_def = boolean_feature | numerical_feature;

const auto positive_condition_def = keyword(runir::kr::ps::dl::Positive::keyword) >> attr(ast::Positive {});
const auto negative_condition_def = keyword(runir::kr::ps::dl::Negative::keyword) >> attr(ast::Negative {});
const auto equal_zero_condition_def = keyword(runir::kr::ps::dl::EqualZero::keyword) >> attr(ast::EqualZero {});
const auto greater_zero_condition_def = keyword(runir::kr::ps::dl::GreaterZero::keyword) >> attr(ast::GreaterZero {});
const auto condition_observation_def = positive_condition | negative_condition | equal_zero_condition | greater_zero_condition;
const auto condition_def = context("condition")[(lit("(") >> condition_observation) > identifier > lit(")")];

const auto positive_effect_def = keyword(runir::kr::ps::dl::Positive::keyword) >> attr(ast::Positive {});
const auto negative_effect_def = keyword(runir::kr::ps::dl::Negative::keyword) >> attr(ast::Negative {});
const auto unchanged_effect_def = keyword(runir::kr::ps::dl::Unchanged::keyword) >> attr(ast::Unchanged {});
const auto increases_effect_def = keyword(runir::kr::ps::dl::Increases::keyword) >> attr(ast::Increases {});
const auto decreases_effect_def = keyword(runir::kr::ps::dl::Decreases::keyword) >> attr(ast::Decreases {});
const auto effect_observation_def = positive_effect | negative_effect | unchanged_effect | increases_effect | decreases_effect;
const auto effect_def = context("effect")[(lit("(") >> effect_observation) > identifier > lit(")")];

const auto conditions_section_def = context(":conditions")[(lit("(") >> keyword(":conditions")) > *condition > lit(")")];
const auto effects_section_def = context(":effects")[(lit("(") >> keyword(":effects")) > *effect > lit(")")];
const auto rule_expression_section_def =
    context("rule expression")[(lit("(") >> keyword(":expression")) > conditions_section_def > effects_section_def > lit(")")];
const auto features_section_def = context(":features")[(lit("(") >> keyword(":features")) > *feature > lit(")")];
const auto rules_section_def = context(":rules")[(lit("(") >> keyword(":rules")) > *rule > lit(")")];

const auto rule_def = context("rule")[(lit("(") >> keyword(":rule")) > symbol_section_def > rule_expression_section_def > lit(")")];
const auto sketch_def = context("sketch")[(lit("(") >> keyword(":sketch")) > features_section_def > rules_section_def > lit(")")];
const auto sketch_root_def = context("sketch")[sketch > eoi];

BOOST_SPIRIT_DEFINE(identifier, boolean_feature, numerical_feature, feature)
BOOST_SPIRIT_DEFINE(positive_condition, negative_condition, equal_zero_condition, greater_zero_condition, condition_observation, condition)
BOOST_SPIRIT_DEFINE(positive_effect, negative_effect, unchanged_effect, increases_effect, decreases_effect, effect_observation, effect)
BOOST_SPIRIT_DEFINE(rule, sketch, sketch_root)

struct IdentifierClass : runir::kr::parser::ErrorHandlerBase
{
};

struct BooleanFeatureClass : boost::spirit::x3::annotate_on_success
{
};
struct NumericalFeatureClass : boost::spirit::x3::annotate_on_success
{
};
struct FeatureClass : boost::spirit::x3::annotate_on_success
{
};

struct PositiveConditionClass : boost::spirit::x3::annotate_on_success
{
};
struct NegativeConditionClass : boost::spirit::x3::annotate_on_success
{
};
struct EqualZeroConditionClass : boost::spirit::x3::annotate_on_success
{
};
struct GreaterZeroConditionClass : boost::spirit::x3::annotate_on_success
{
};
struct ConditionObservationClass : boost::spirit::x3::annotate_on_success
{
};
struct ConditionClass : boost::spirit::x3::annotate_on_success
{
};

struct PositiveEffectClass : boost::spirit::x3::annotate_on_success
{
};
struct NegativeEffectClass : boost::spirit::x3::annotate_on_success
{
};
struct UnchangedEffectClass : boost::spirit::x3::annotate_on_success
{
};
struct IncreasesEffectClass : boost::spirit::x3::annotate_on_success
{
};
struct DecreasesEffectClass : boost::spirit::x3::annotate_on_success
{
};
struct EffectObservationClass : boost::spirit::x3::annotate_on_success
{
};
struct EffectClass : boost::spirit::x3::annotate_on_success
{
};

struct RuleClass : boost::spirit::x3::annotate_on_success
{
};
struct SketchClass : boost::spirit::x3::annotate_on_success
{
};
struct SketchRootClass : runir::kr::parser::ErrorHandlerBase
{
};

}  // namespace runir::kr::ps::base::dl::parser

#endif
