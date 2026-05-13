#ifndef RUNIR_SRC_KR_GP_DL_PARSER_PARSER_DEF_HPP_
#define RUNIR_SRC_KR_GP_DL_PARSER_PARSER_DEF_HPP_

#include "runir/kr/dl/grammar/parser/parsers.hpp"
#include "runir/kr/gp/dl/ast/ast.hpp"
#include "runir/kr/gp/dl/ast/ast_adapted.hpp"
#include "runir/kr/gp/dl/parser/error_handler.hpp"
#include "runir/kr/gp/dl/parser/parsers.hpp"

#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>

namespace runir::kr::gp::dl::parser
{
namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;
namespace dl_parser = runir::kr::dl::grammar::parser;

using x3::attr;
using x3::eoi;
using x3::lexeme;
using x3::lit;
using x3::raw;

using ascii::alnum;
using ascii::alpha;
using ascii::char_;

boolean_feature_type const boolean_feature = "boolean_feature";
numerical_feature_type const numerical_feature = "numerical_feature";
feature_type const feature = "feature";

positive_boolean_condition_type const positive_boolean_condition = "positive_boolean_condition";
negative_boolean_condition_type const negative_boolean_condition = "negative_boolean_condition";
equal_zero_numerical_condition_type const equal_zero_numerical_condition = "equal_zero_numerical_condition";
greater_zero_numerical_condition_type const greater_zero_numerical_condition = "greater_zero_numerical_condition";
condition_observation_type const condition_observation = "condition_observation";
condition_type const condition = "condition";

positive_boolean_effect_type const positive_boolean_effect = "positive_boolean_effect";
negative_boolean_effect_type const negative_boolean_effect = "negative_boolean_effect";
unchanged_effect_type const unchanged_effect = "unchanged_effect";
increases_numerical_effect_type const increases_numerical_effect = "increases_numerical_effect";
decreases_numerical_effect_type const decreases_numerical_effect = "decreases_numerical_effect";
effect_observation_type const effect_observation = "effect_observation";
effect_type const effect = "effect";

rule_type const rule = "rule";
policy_type const policy = "policy";
policy_root_type const policy_root = "policy_root";

inline auto identifier_parser() { return raw[lexeme[(alpha | char_('_')) >> *(alnum | char_('_') | char_('-'))]]; }

inline auto quoted_string_parser() { return lexeme[lit('"') >> raw[*('\\' >> char_ | (char_ - '"'))] >> lit('"')]; }

const auto boolean_feature_def = lit("(") >> lit("boolean") > identifier_parser() > quoted_string_parser() > quoted_string_parser()
                                 > dl_parser::boolean_parser() > lit(")");
const auto numerical_feature_def = lit("(") >> lit("numerical") > identifier_parser() > quoted_string_parser() > quoted_string_parser()
                                   > dl_parser::numerical_parser() > lit(")");
const auto feature_def = boolean_feature | numerical_feature;

const auto positive_boolean_condition_def = lit("positive") >> attr(Positive {});
const auto negative_boolean_condition_def = lit("negative") >> attr(Negative {});
const auto equal_zero_numerical_condition_def = lit("equal_zero") >> attr(EqualZero {});
const auto greater_zero_numerical_condition_def = lit("greater_zero") >> attr(GreaterZero {});
const auto condition_observation_def =
    positive_boolean_condition | negative_boolean_condition | equal_zero_numerical_condition | greater_zero_numerical_condition;
const auto condition_def = lit("(") >> condition_observation > identifier_parser() > lit(")");

const auto positive_boolean_effect_def = lit("positive") >> attr(Positive {});
const auto negative_boolean_effect_def = lit("negative") >> attr(Negative {});
const auto unchanged_effect_def = lit("unchanged") >> attr(Unchanged {});
const auto increases_numerical_effect_def = lit("increases") >> attr(Increases {});
const auto decreases_numerical_effect_def = lit("decreases") >> attr(Decreases {});
const auto effect_observation_def =
    positive_boolean_effect | negative_boolean_effect | unchanged_effect | increases_numerical_effect | decreases_numerical_effect;
const auto effect_def = lit("(") >> effect_observation > identifier_parser() > lit(")");

const auto conditions_section_def = lit("(") > lit(":conditions") > *condition > lit(")");
const auto effects_section_def = lit("(") > lit(":effects") > *effect > lit(")");
const auto features_section_def = lit("(") > lit(":features") > *feature > lit(")");
const auto rules_section_def = lit("(") > lit(":rules") > *rule > lit(")");

const auto rule_def = lit("(") > conditions_section_def > effects_section_def > lit(")");
const auto policy_def = lit("(") > -lit("policy") > features_section_def > rules_section_def > lit(")");
const auto policy_root_def = policy > eoi;

BOOST_SPIRIT_DEFINE(boolean_feature, numerical_feature, feature)
BOOST_SPIRIT_DEFINE(positive_boolean_condition,
                    negative_boolean_condition,
                    equal_zero_numerical_condition,
                    greater_zero_numerical_condition,
                    condition_observation,
                    condition)
BOOST_SPIRIT_DEFINE(positive_boolean_effect,
                    negative_boolean_effect,
                    unchanged_effect,
                    increases_numerical_effect,
                    decreases_numerical_effect,
                    effect_observation,
                    effect)
BOOST_SPIRIT_DEFINE(rule, policy, policy_root)

struct BooleanFeatureClass : x3::annotate_on_success
{
};
struct NumericalFeatureClass : x3::annotate_on_success
{
};
struct FeatureClass : x3::annotate_on_success
{
};

struct PositiveBooleanConditionClass : x3::annotate_on_success
{
};
struct NegativeBooleanConditionClass : x3::annotate_on_success
{
};
struct EqualZeroNumericalConditionClass : x3::annotate_on_success
{
};
struct GreaterZeroNumericalConditionClass : x3::annotate_on_success
{
};
struct ConditionObservationClass : x3::annotate_on_success
{
};
struct ConditionClass : x3::annotate_on_success
{
};

struct PositiveBooleanEffectClass : x3::annotate_on_success
{
};
struct NegativeBooleanEffectClass : x3::annotate_on_success
{
};
struct UnchangedEffectClass : x3::annotate_on_success
{
};
struct IncreasesNumericalEffectClass : x3::annotate_on_success
{
};
struct DecreasesNumericalEffectClass : x3::annotate_on_success
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
struct PolicyClass : x3::annotate_on_success
{
};
struct PolicyRootClass : ErrorHandlerBase
{
};

boolean_feature_type const& boolean_feature_parser() { return boolean_feature; }
numerical_feature_type const& numerical_feature_parser() { return numerical_feature; }
feature_type const& feature_parser() { return feature; }

positive_boolean_condition_type const& positive_boolean_condition_parser() { return positive_boolean_condition; }
negative_boolean_condition_type const& negative_boolean_condition_parser() { return negative_boolean_condition; }
equal_zero_numerical_condition_type const& equal_zero_numerical_condition_parser() { return equal_zero_numerical_condition; }
greater_zero_numerical_condition_type const& greater_zero_numerical_condition_parser() { return greater_zero_numerical_condition; }
condition_observation_type const& condition_observation_parser() { return condition_observation; }
condition_type const& condition_parser() { return condition; }

positive_boolean_effect_type const& positive_boolean_effect_parser() { return positive_boolean_effect; }
negative_boolean_effect_type const& negative_boolean_effect_parser() { return negative_boolean_effect; }
unchanged_effect_type const& unchanged_effect_parser() { return unchanged_effect; }
increases_numerical_effect_type const& increases_numerical_effect_parser() { return increases_numerical_effect; }
decreases_numerical_effect_type const& decreases_numerical_effect_parser() { return decreases_numerical_effect; }
effect_observation_type const& effect_observation_parser() { return effect_observation; }
effect_type const& effect_parser() { return effect; }

rule_type const& rule_parser() { return rule; }
policy_type const& policy_parser() { return policy; }
policy_root_type const& policy_root_parser() { return policy_root; }

}  // namespace runir::kr::gp::dl::parser

#endif
