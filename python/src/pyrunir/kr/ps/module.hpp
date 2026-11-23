#ifndef PYRUNIR_KR_PS_MODULE_HPP_
#define PYRUNIR_KR_PS_MODULE_HPP_

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace runir::kr::ps
{

void bind_module_definitions(nb::module_& m);

/// The serialization hooks of the types this module binds (pyyggdrasil.serialization), once they are bound.
void bind_execution_serialization();

}  // namespace runir::kr::ps

#endif
