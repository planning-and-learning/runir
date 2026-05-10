#ifndef PYRUNIR_GRAPHS_MODULE_HPP_
#define PYRUNIR_GRAPHS_MODULE_HPP_

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace runir::graphs
{

void bind_module_definitions(nb::module_& m);

}  // namespace runir::graphs

#endif
