#ifndef PYRUNIR_KR_DL_UNS_MODULE_HPP_
#define PYRUNIR_KR_DL_UNS_MODULE_HPP_

#include "../module.hpp"

namespace runir::kr::dl::uns
{

void bind_module_definitions(nb::module_& m);
void bind_indices(nb::module_& m);
void bind_parser(nb::module_& m);
void bind_repository(nb::module_& m);
void bind_semantics_module_definitions(nb::module_& m);

}  // namespace runir::kr::dl::uns

#endif
