#ifndef PYRUNIR_KR_UNS_DL_MODULE_HPP_
#define PYRUNIR_KR_UNS_DL_MODULE_HPP_

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace runir::kr::uns::dl
{

void bind_module_definitions(nb::module_& m);
void bind_indices(nb::module_& m);
void bind_views(nb::module_& m);
void bind_parser(nb::module_& m);
void bind_classifier_factory(nb::module_& m);

}  // namespace runir::kr::uns::dl

#endif
