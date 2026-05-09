#ifndef PYRUNIR_KR_DL_DL_HPP_
#define PYRUNIR_KR_DL_DL_HPP_

#include <nanobind/nanobind.h>

namespace runir::kr::dl
{

namespace nb = nanobind;

void bind_module_definitions(nb::module_& m);
void bind_indices(nb::module_& m);
void bind_datas(nb::module_& m);
void bind_views(nb::module_& m);
void bind_repositories(nb::module_& m);
void bind_grammar_module_definitions(nb::module_& m);
void bind_cnf_grammar_module_definitions(nb::module_& m);

}

#endif
