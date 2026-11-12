#ifndef PYRUNIR_KR_DL_SEMANTICS_BINDINGS_HPP_
#define PYRUNIR_KR_DL_SEMANTICS_BINDINGS_HPP_

#include "module.hpp"

namespace runir::kr::dl
{

void bind_semantics_denotation(nb::module_& m);
void bind_semantics_query(nb::module_& m);
void bind_semantics_records(nb::module_& m);
void bind_semantics_feature_records(nb::module_& m);
void bind_semantics_constructor(nb::module_& m);
void bind_semantics_repositories(nb::module_& m);

}  // namespace runir::kr::dl

#endif
