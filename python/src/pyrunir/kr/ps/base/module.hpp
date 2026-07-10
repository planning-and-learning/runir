#ifndef PYRUNIR_KR_PS_BASE_MODULE_HPP_
#define PYRUNIR_KR_PS_BASE_MODULE_HPP_

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace runir::kr::ps::base
{

void bind_module_definitions(nb::module_& m);
void bind_indices(nb::module_& m);
void bind_datas(nb::module_& m);
void bind_views(nb::module_& m);
void bind_repository(nb::module_& m);
void bind_sketch_executor(nb::module_& m);
void bind_syntactic_complexity(nb::module_& m);

}  // namespace runir::kr::ps::base

#endif
