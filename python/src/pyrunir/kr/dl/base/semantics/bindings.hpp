#ifndef PYRUNIR_KR_DL_BASE_SEMANTICS_BINDINGS_HPP_
#define PYRUNIR_KR_DL_BASE_SEMANTICS_BINDINGS_HPP_

#include "module.hpp"

namespace runir::kr::dl::base
{

void bind_semantics_denotation(nb::module_& m);
void bind_semantics_concept(nb::module_& m);
void bind_semantics_role(nb::module_& m);
void bind_semantics_boolean(nb::module_& m);
void bind_semantics_numerical(nb::module_& m);
void bind_semantics_constructor(nb::module_& m);
void bind_semantics_repositories(nb::module_& m);

}  // namespace runir::kr::dl::base

#endif
