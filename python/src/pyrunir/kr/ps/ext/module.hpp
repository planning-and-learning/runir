#ifndef PYRUNIR_KR_PS_EXT_MODULE_HPP_
#define PYRUNIR_KR_PS_EXT_MODULE_HPP_

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace runir::kr::ps::ext
{

void bind_module_definitions(nb::module_& m);
void bind_datas(nb::module_& m);
void bind_indices(nb::module_& m);
void bind_views(nb::module_& m);
void bind_repository(nb::module_& m);
void bind_module_program_executor(nb::module_& m);
void bind_module_factory(nb::module_& m);
void bind_parser(nb::module_& m);

}  // namespace runir::kr::ps::ext

#endif
