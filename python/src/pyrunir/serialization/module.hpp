#ifndef PYRUNIR_SERIALIZATION_MODULE_HPP_
#define PYRUNIR_SERIALIZATION_MODULE_HPP_

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace runir::serialization
{

void bind_module_definitions(nb::module_& m);

}  // namespace runir::serialization

#endif
