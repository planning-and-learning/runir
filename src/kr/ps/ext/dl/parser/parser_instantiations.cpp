#include "runir/kr/ps/ext/dl/parser/parser_def.hpp"

namespace runir::kr::ps::ext::dl::parser
{

using iterator_type = runir::kr::parser::Iterator;
using context_type = runir::kr::parser::Context;

BOOST_SPIRIT_INSTANTIATE(module_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(program_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(module_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(program_root_type, iterator_type, context_type)

module_type const& module_parser() { return module; }
program_type const& program_parser() { return program; }
module_root_type const& module_root_parser() { return module_root; }
program_root_type const& program_root_parser() { return program_root; }

}  // namespace runir::kr::ps::ext::dl::parser
