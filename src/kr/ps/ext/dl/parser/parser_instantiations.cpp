#include "parser_def.hpp"

namespace runir::kr::ps::ext::dl::parser
{

BOOST_SPIRIT_INSTANTIATE(module_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(module_program_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(module_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(module_program_root_type, iterator_type, context_type)

}  // namespace runir::kr::ps::ext::dl::parser
