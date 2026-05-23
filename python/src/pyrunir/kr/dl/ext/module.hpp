#ifndef PYRUNIR_KR_DL_EXT_MODULE_HPP_
#define PYRUNIR_KR_DL_EXT_MODULE_HPP_

#include "../module.hpp"

namespace runir::kr::dl::ext
{

void bind_module_definitions(nb::module_& m);
void bind_indices(nb::module_& m);
void bind_datas(nb::module_& m);
void bind_parser(nb::module_& m);

}  // namespace runir::kr::dl::ext

#endif
