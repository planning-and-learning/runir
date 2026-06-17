#include "parser_def.hpp"

namespace runir::kr::uns::dl::parser
{

BOOST_SPIRIT_INSTANTIATE(boolean_feature_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dnf_literal_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dnf_clause_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(dnf_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(classifier_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(classifier_root_type, iterator_type, context_type)

}  // namespace runir::kr::uns::dl::parser
