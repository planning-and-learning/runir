#include "parser_def.hpp"

namespace runir::kr::gp::dl::parser
{

BOOST_SPIRIT_INSTANTIATE(boolean_feature_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(numerical_feature_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(feature_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(positive_condition_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(negative_condition_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(equal_zero_condition_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(greater_zero_condition_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(condition_observation_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(condition_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(positive_effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(negative_effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(unchanged_effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(increases_effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(decreases_effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(effect_observation_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(effect_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(rule_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(policy_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(policy_root_type, iterator_type, context_type)

}  // namespace runir::kr::gp::dl::parser
