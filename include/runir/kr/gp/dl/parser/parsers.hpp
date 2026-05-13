#ifndef RUNIR_KR_GP_DL_PARSER_PARSERS_HPP_
#define RUNIR_KR_GP_DL_PARSER_PARSERS_HPP_

#include "runir/kr/gp/dl/ast/ast.hpp"

#include <boost/spirit/home/x3.hpp>

namespace runir::kr::gp::dl::parser
{
namespace x3 = boost::spirit::x3;

struct BooleanFeatureClass;
struct NumericalFeatureClass;
struct FeatureClass;

struct PositiveConditionClass;
struct NegativeConditionClass;
struct EqualZeroConditionClass;
struct GreaterZeroConditionClass;
struct ConditionObservationClass;
struct ConditionClass;

struct PositiveEffectClass;
struct NegativeEffectClass;
struct UnchangedEffectClass;
struct IncreasesEffectClass;
struct DecreasesEffectClass;
struct EffectObservationClass;
struct EffectClass;

struct RuleClass;
struct PolicyClass;
struct PolicyRootClass;

using boolean_feature_type = x3::rule<BooleanFeatureClass, ast::BooleanFeature>;
using numerical_feature_type = x3::rule<NumericalFeatureClass, ast::NumericalFeature>;
using feature_type = x3::rule<FeatureClass, ast::Feature>;

using positive_condition_type = x3::rule<PositiveConditionClass, Positive>;
using negative_condition_type = x3::rule<NegativeConditionClass, Negative>;
using equal_zero_condition_type = x3::rule<EqualZeroConditionClass, EqualZero>;
using greater_zero_condition_type = x3::rule<GreaterZeroConditionClass, GreaterZero>;
using condition_observation_type = x3::rule<ConditionObservationClass, ast::ConditionObservation>;
using condition_type = x3::rule<ConditionClass, ast::Condition>;

using positive_effect_type = x3::rule<PositiveEffectClass, Positive>;
using negative_effect_type = x3::rule<NegativeEffectClass, Negative>;
using unchanged_effect_type = x3::rule<UnchangedEffectClass, Unchanged>;
using increases_effect_type = x3::rule<IncreasesEffectClass, Increases>;
using decreases_effect_type = x3::rule<DecreasesEffectClass, Decreases>;
using effect_observation_type = x3::rule<EffectObservationClass, ast::EffectObservation>;
using effect_type = x3::rule<EffectClass, ast::Effect>;

using rule_type = x3::rule<RuleClass, ast::Rule>;
using policy_type = x3::rule<PolicyClass, ast::Policy>;
using policy_root_type = x3::rule<PolicyRootClass, ast::Policy>;

BOOST_SPIRIT_DECLARE(boolean_feature_type, numerical_feature_type, feature_type)
BOOST_SPIRIT_DECLARE(positive_condition_type,
                     negative_condition_type,
                     equal_zero_condition_type,
                     greater_zero_condition_type,
                     condition_observation_type,
                     condition_type)
BOOST_SPIRIT_DECLARE(positive_effect_type,
                     negative_effect_type,
                     unchanged_effect_type,
                     increases_effect_type,
                     decreases_effect_type,
                     effect_observation_type,
                     effect_type)
BOOST_SPIRIT_DECLARE(rule_type, policy_type, policy_root_type)

boolean_feature_type const& boolean_feature_parser();
numerical_feature_type const& numerical_feature_parser();
feature_type const& feature_parser();

positive_condition_type const& positive_condition_parser();
negative_condition_type const& negative_condition_parser();
equal_zero_condition_type const& equal_zero_condition_parser();
greater_zero_condition_type const& greater_zero_condition_parser();
condition_observation_type const& condition_observation_parser();
condition_type const& condition_parser();

positive_effect_type const& positive_effect_parser();
negative_effect_type const& negative_effect_parser();
unchanged_effect_type const& unchanged_effect_parser();
increases_effect_type const& increases_effect_parser();
decreases_effect_type const& decreases_effect_parser();
effect_observation_type const& effect_observation_parser();
effect_type const& effect_parser();

rule_type const& rule_parser();
policy_type const& policy_parser();
policy_root_type const& policy_root_parser();

}  // namespace runir::kr::gp::dl::parser

#endif
