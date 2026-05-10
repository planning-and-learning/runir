#ifndef PYRUNIR_KR_DL_MODULE_HPP_
#define PYRUNIR_KR_DL_MODULE_HPP_

#include <nanobind/nanobind.h>

namespace runir::kr::dl
{

namespace nb = nanobind;

void bind_module_definitions(nb::module_& m);

}

#endif
