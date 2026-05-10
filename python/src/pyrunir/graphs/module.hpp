#ifndef PYRUNIR_GRAPHS_MODULE_HPP_
#define PYRUNIR_GRAPHS_MODULE_HPP_

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace runir::python
{

void bind_graphs(nb::module_& m);

}  // namespace runir::python

#endif
