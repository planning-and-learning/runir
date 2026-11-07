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

base_sketch_root_type const& sketch_root_parser() { return sketch_root; }

}  // namespace runir::kr::ps::base::dl::parser
