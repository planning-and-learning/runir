#include "parser_def.hpp"

namespace runir::kr::ps::base::dl::parser
{

BOOST_SPIRIT_INSTANTIATE(base_boolean_feature_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_numerical_feature_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_feature_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(positive_condition_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(negative_condition_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(equal_zero_condition_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(greater_zero_condition_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_condition_observation_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_condition_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(positive_effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(negative_effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(unchanged_effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(increases_effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(decreases_effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_effect_observation_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_effect_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(base_rule_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_sketch_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_sketch_root_type, iterator_type, context_type)

}  // namespace runir::kr::ps::base::dl::parser
