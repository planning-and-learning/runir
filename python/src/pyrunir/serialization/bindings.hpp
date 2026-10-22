#ifndef PYRUNIR_SERIALIZATION_BINDINGS_HPP_
#define PYRUNIR_SERIALIZATION_BINDINGS_HPP_

#include "module.hpp"

namespace runir::serialization
{

void bind_kr_dl_concept(nb::module_& m);
void bind_kr_dl_role(nb::module_& m);
void bind_kr_dl_boolean(nb::module_& m);
void bind_kr_dl_numerical(nb::module_& m);
void bind_kr_dl_constructor(nb::module_& m);
void bind_kr_dl_query(nb::module_& m);
void bind_kr_dl_denotation(nb::module_& m);
void bind_kr_ps_base(nb::module_& m);
void bind_kr_ps_ext(nb::module_& m);
void bind_kr_ps_icp(nb::module_& m);
void bind_kr_uns(nb::module_& m);
void bind_kr_execution(nb::module_& m);
void bind_graphs(nb::module_& m);

}  // namespace runir::serialization

#endif
