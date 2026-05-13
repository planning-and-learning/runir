#ifndef PYRUNIR_DATASETS_MODULE_HPP_
#define PYRUNIR_DATASETS_MODULE_HPP_

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace runir::datasets
{

void bind_module_definitions(nb::module_& m);
void bind_config(nb::module_& m);
void bind_task_class(nb::module_& m);
void bind_state_graph(nb::module_& m);
void bind_equivalence_graph(nb::module_& m);

}  // namespace runir::datasets

#endif
