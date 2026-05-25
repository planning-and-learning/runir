#include "parser_def.hpp"

namespace runir::kr::dl::grammar::parser::base
{

BOOST_SPIRIT_INSTANTIATE(base_concept_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_concept_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_role_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_role_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_concept_non_terminal_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_role_non_terminal_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_concept_choice_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_role_choice_type, iterator_type, context_type)

}  // namespace runir::kr::dl::grammar::parser::base
