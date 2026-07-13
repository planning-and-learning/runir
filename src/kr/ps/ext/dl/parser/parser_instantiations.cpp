#include "runir/kr/ps/ext/dl/parser/parser_def.hpp"

namespace runir::kr::ps::ext::dl::parser
{

using iterator_type = runir::kr::parser::Iterator;
using context_type = runir::kr::parser::Context;

BOOST_SPIRIT_INSTANTIATE(module_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(module_program_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(module_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(module_program_root_type, iterator_type, context_type)

module_type const& module_parser() { return module; }
module_program_type const& module_program_parser() { return module_program; }
module_root_type const& module_root_parser() { return module_root; }
module_program_root_type const& module_program_root_parser() { return module_program_root; }

}  // namespace runir::kr::ps::ext::dl::parser
