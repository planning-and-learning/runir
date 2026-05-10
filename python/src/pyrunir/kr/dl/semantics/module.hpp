#ifndef PYRUNIR_KR_DL_SEMANTICS_MODULE_HPP_
#define PYRUNIR_KR_DL_SEMANTICS_MODULE_HPP_

#include "../module.hpp"

namespace runir::kr::dl
{

void bind_semantics_module_definitions(nb::module_& m);
void bind_semantics_indices(nb::module_& m);
void bind_semantics_datas(nb::module_& m);
void bind_semantics_views(nb::module_& m);
void bind_semantics_repositories(nb::module_& m);

}

#endif
