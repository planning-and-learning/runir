#ifndef RUNIR_KR_PS_BASE_DL_PARSER_PARSERS_HPP_
#define RUNIR_KR_PS_BASE_DL_PARSER_PARSERS_HPP_

#include "runir/kr/ps/base/dl/ast/ast.hpp"

#include <boost/spirit/home/x3.hpp>

namespace runir::kr::ps::base::dl::parser
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
struct SketchClass;
struct SketchRootClass;

using base_boolean_feature_type = x3::rule<BooleanFeatureClass, ast::BooleanFeature<runir::kr::BaseFamilyTag>>;
using base_numerical_feature_type = x3::rule<NumericalFeatureClass, ast::NumericalFeature<runir::kr::BaseFamilyTag>>;
using base_feature_type = x3::rule<FeatureClass, ast::Feature<runir::kr::BaseFamilyTag>>;

using positive_condition_type = x3::rule<PositiveConditionClass, ast::Positive>;
using negative_condition_type = x3::rule<NegativeConditionClass, ast::Negative>;
using equal_zero_condition_type = x3::rule<EqualZeroConditionClass, ast::EqualZero>;
using greater_zero_condition_type = x3::rule<GreaterZeroConditionClass, ast::GreaterZero>;
using base_condition_observation_type = x3::rule<ConditionObservationClass, ast::ConditionObservation<runir::kr::BaseFamilyTag>>;
using base_condition_type = x3::rule<ConditionClass, ast::Condition<runir::kr::BaseFamilyTag>>;

using positive_effect_type = x3::rule<PositiveEffectClass, ast::Positive>;
using negative_effect_type = x3::rule<NegativeEffectClass, ast::Negative>;
using unchanged_effect_type = x3::rule<UnchangedEffectClass, ast::Unchanged>;
using increases_effect_type = x3::rule<IncreasesEffectClass, ast::Increases>;
using decreases_effect_type = x3::rule<DecreasesEffectClass, ast::Decreases>;
using base_effect_observation_type = x3::rule<EffectObservationClass, ast::EffectObservation<runir::kr::BaseFamilyTag>>;
using base_effect_type = x3::rule<EffectClass, ast::Effect<runir::kr::BaseFamilyTag>>;

using base_rule_type = x3::rule<RuleClass, ast::Rule<runir::kr::BaseFamilyTag>>;
using base_sketch_type = x3::rule<SketchClass, ast::Sketch<runir::kr::BaseFamilyTag>>;
using base_sketch_root_type = x3::rule<SketchRootClass, ast::Sketch<runir::kr::BaseFamilyTag>>;

BOOST_SPIRIT_DECLARE(base_boolean_feature_type, base_numerical_feature_type, base_feature_type)
BOOST_SPIRIT_DECLARE(positive_condition_type,
                     negative_condition_type,
                     equal_zero_condition_type,
                     greater_zero_condition_type,
                     base_condition_observation_type,
                     base_condition_type)
BOOST_SPIRIT_DECLARE(positive_effect_type,
                     negative_effect_type,
                     unchanged_effect_type,
                     increases_effect_type,
                     decreases_effect_type,
                     base_effect_observation_type,
                     base_effect_type)
BOOST_SPIRIT_DECLARE(base_rule_type, base_sketch_type, base_sketch_root_type)

base_boolean_feature_type const& boolean_feature_parser();
base_numerical_feature_type const& numerical_feature_parser();
base_feature_type const& feature_parser();

positive_condition_type const& positive_condition_parser();
negative_condition_type const& negative_condition_parser();
equal_zero_condition_type const& equal_zero_condition_parser();
greater_zero_condition_type const& greater_zero_condition_parser();
base_condition_observation_type const& condition_observation_parser();
base_condition_type const& condition_parser();

positive_effect_type const& positive_effect_parser();
negative_effect_type const& negative_effect_parser();
unchanged_effect_type const& unchanged_effect_parser();
increases_effect_type const& increases_effect_parser();
decreases_effect_type const& decreases_effect_parser();
base_effect_observation_type const& effect_observation_parser();
base_effect_type const& effect_parser();

base_rule_type const& rule_parser();
base_sketch_type const& sketch_parser();
base_sketch_root_type const& sketch_root_parser();

}  // namespace runir::kr::ps::base::dl::parser

#endif
