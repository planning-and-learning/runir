#include "runir/kr/ps/base/dl/parser/parser_def.hpp"

namespace runir::kr::ps::base::dl::parser
{

BOOST_SPIRIT_INSTANTIATE(base_boolean_feature_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(base_numerical_feature_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(base_feature_type, runir::kr::parser::Iterator, runir::kr::parser::Context)

BOOST_SPIRIT_INSTANTIATE(positive_condition_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(negative_condition_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(equal_zero_condition_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(greater_zero_condition_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(base_condition_observation_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(base_condition_type, runir::kr::parser::Iterator, runir::kr::parser::Context)

BOOST_SPIRIT_INSTANTIATE(positive_effect_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(negative_effect_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(unchanged_effect_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(increases_effect_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(decreases_effect_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(base_effect_observation_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(base_effect_type, runir::kr::parser::Iterator, runir::kr::parser::Context)

BOOST_SPIRIT_INSTANTIATE(base_rule_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(base_sketch_type, runir::kr::parser::Iterator, runir::kr::parser::Context)
BOOST_SPIRIT_INSTANTIATE(base_sketch_root_type, runir::kr::parser::Iterator, runir::kr::parser::Context)

base_boolean_feature_type const& boolean_feature_parser() { return boolean_feature; }
base_numerical_feature_type const& numerical_feature_parser() { return numerical_feature; }
base_feature_type const& feature_parser() { return feature; }

positive_condition_type const& positive_condition_parser() { return positive_condition; }
negative_condition_type const& negative_condition_parser() { return negative_condition; }
equal_zero_condition_type const& equal_zero_condition_parser() { return equal_zero_condition; }
greater_zero_condition_type const& greater_zero_condition_parser() { return greater_zero_condition; }
base_condition_observation_type const& condition_observation_parser() { return condition_observation; }
base_condition_type const& condition_parser() { return condition; }

positive_effect_type const& positive_effect_parser() { return positive_effect; }
negative_effect_type const& negative_effect_parser() { return negative_effect; }
unchanged_effect_type const& unchanged_effect_parser() { return unchanged_effect; }
increases_effect_type const& increases_effect_parser() { return increases_effect; }
decreases_effect_type const& decreases_effect_parser() { return decreases_effect; }
base_effect_observation_type const& effect_observation_parser() { return effect_observation; }
base_effect_type const& effect_parser() { return effect; }

base_rule_type const& rule_parser() { return rule; }
base_sketch_type const& sketch_parser() { return sketch; }
base_sketch_root_type const& sketch_root_parser() { return sketch_root; }

}  // namespace runir::kr::ps::base::dl::parser
