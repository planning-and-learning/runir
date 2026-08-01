#ifndef PYRUNIR_KR_UNS_MODULE_HPP_
#define PYRUNIR_KR_UNS_MODULE_HPP_

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace runir::kr::uns
{

void bind_module_definitions(nb::module_& m);

}  // namespace runir::kr::uns

#endif
