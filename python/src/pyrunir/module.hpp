#ifndef PYRUNIR_MODULE_HPP_
#define PYRUNIR_MODULE_HPP_

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace runir
{

void bind_module_definitions(nb::module_& m);

}  // namespace runir

#endif
