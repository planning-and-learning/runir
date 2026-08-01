#ifndef PYRUNIR_KR_PS_EXT_DL_MODULE_HPP_
#define PYRUNIR_KR_PS_EXT_DL_MODULE_HPP_

#include <nanobind/nanobind.h>

#include "../bindings.hpp"

namespace nb = nanobind;

namespace runir::kr::ps::ext::dl
{

void bind_module_definitions(nb::module_& m, RepositoryBinding& repository);

}  // namespace runir::kr::ps::ext::dl

#endif
