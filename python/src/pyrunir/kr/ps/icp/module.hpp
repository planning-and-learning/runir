#ifndef PYRUNIR_KR_PS_ICP_MODULE_HPP_
#define PYRUNIR_KR_PS_ICP_MODULE_HPP_

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace runir::kr::ps::icp
{

void bind_module_definitions(nb::module_& m);

}  // namespace runir::kr::ps::icp

#endif
