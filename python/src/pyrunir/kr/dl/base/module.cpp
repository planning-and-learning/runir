#include "module.hpp"

namespace runir::kr::dl::base
{

void bind_module_definitions(nb::module_& m) { bind_parser(m); }

}  // namespace runir::kr::dl::base
