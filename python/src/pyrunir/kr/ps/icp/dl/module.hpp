#ifndef PYRUNIR_KR_PS_ICP_DL_MODULE_HPP_
#define PYRUNIR_KR_PS_ICP_DL_MODULE_HPP_

#include "../bindings.hpp"

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace runir::kr::ps::icp::dl
{

void bind_module_definitions(nb::module_& m, RepositoryBinding& repository);

}  // namespace runir::kr::ps::icp::dl

#endif
