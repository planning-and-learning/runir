#include "parser_def.hpp"

namespace runir::kr::dl::grammar::parser::ext
{

BOOST_SPIRIT_INSTANTIATE(ext_concept_type, runir::kr::dl::grammar::parser::iterator_type, runir::kr::dl::grammar::parser::context_type)
BOOST_SPIRIT_INSTANTIATE(ext_concept_root_type, runir::kr::dl::grammar::parser::iterator_type, runir::kr::dl::grammar::parser::context_type)
BOOST_SPIRIT_INSTANTIATE(ext_role_type, runir::kr::dl::grammar::parser::iterator_type, runir::kr::dl::grammar::parser::context_type)
BOOST_SPIRIT_INSTANTIATE(ext_role_root_type, runir::kr::dl::grammar::parser::iterator_type, runir::kr::dl::grammar::parser::context_type)
BOOST_SPIRIT_INSTANTIATE(ext_concept_choice_type, runir::kr::dl::grammar::parser::iterator_type, runir::kr::dl::grammar::parser::context_type)
BOOST_SPIRIT_INSTANTIATE(ext_role_choice_type, runir::kr::dl::grammar::parser::iterator_type, runir::kr::dl::grammar::parser::context_type)

BOOST_SPIRIT_INSTANTIATE(concept_register_type, runir::kr::dl::grammar::parser::iterator_type, runir::kr::dl::grammar::parser::context_type)
BOOST_SPIRIT_INSTANTIATE(role_register_type, runir::kr::dl::grammar::parser::iterator_type, runir::kr::dl::grammar::parser::context_type)
BOOST_SPIRIT_INSTANTIATE(concept_argument_type, runir::kr::dl::grammar::parser::iterator_type, runir::kr::dl::grammar::parser::context_type)
BOOST_SPIRIT_INSTANTIATE(role_argument_type, runir::kr::dl::grammar::parser::iterator_type, runir::kr::dl::grammar::parser::context_type)
BOOST_SPIRIT_INSTANTIATE(boolean_argument_type, runir::kr::dl::grammar::parser::iterator_type, runir::kr::dl::grammar::parser::context_type)
BOOST_SPIRIT_INSTANTIATE(numerical_argument_type, runir::kr::dl::grammar::parser::iterator_type, runir::kr::dl::grammar::parser::context_type)

}  // namespace runir::kr::dl::grammar::parser::ext
