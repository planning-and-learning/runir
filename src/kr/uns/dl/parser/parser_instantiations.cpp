#include "parser_def.hpp"

namespace runir::kr::uns::dl::parser
{

BOOST_SPIRIT_INSTANTIATE(boolean_feature_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(positive_literal_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(negative_literal_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(literal_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(conjunction_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(disjunction_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(classifier_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(classifier_root_type, iterator_type, context_type)

}  // namespace runir::kr::uns::dl::parser
