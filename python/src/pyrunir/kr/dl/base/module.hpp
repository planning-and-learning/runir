#ifndef PYRUNIR_KR_DL_BASE_MODULE_HPP_
#define PYRUNIR_KR_DL_BASE_MODULE_HPP_

#include "../module.hpp"

namespace runir::kr::dl::base
{

void bind_module_definitions(nb::module_& m);
void bind_parser(nb::module_& m);

}  // namespace runir::kr::dl::base

#endif
