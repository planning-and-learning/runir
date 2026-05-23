#include "parser_def.hpp"

namespace runir::kr::dl::grammar::parser::base
{

BOOST_SPIRIT_INSTANTIATE(base_concept_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_concept_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_role_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_role_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_concept_choice_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(base_role_choice_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(ext_concept_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(ext_concept_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(ext_role_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(ext_role_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(ext_concept_choice_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(ext_role_choice_type, iterator_type, context_type)

}  // namespace runir::kr::dl::grammar::parser::base

namespace runir::kr::dl::grammar::parser::ext
{

BOOST_SPIRIT_INSTANTIATE(concept_register_type, runir::kr::dl::grammar::parser::iterator_type, runir::kr::dl::grammar::parser::context_type)

}  // namespace runir::kr::dl::grammar::parser::ext
