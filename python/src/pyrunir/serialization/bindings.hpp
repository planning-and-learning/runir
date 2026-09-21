#ifndef PYRUNIR_SERIALIZATION_BINDINGS_HPP_
#define PYRUNIR_SERIALIZATION_BINDINGS_HPP_

#include "module.hpp"

namespace runir::serialization
{

void bind_kr_dl_base_concept(nb::module_& m);
void bind_kr_dl_base_role(nb::module_& m);
void bind_kr_dl_base_boolean(nb::module_& m);
void bind_kr_dl_base_numerical(nb::module_& m);
void bind_kr_dl_base_constructor(nb::module_& m);
void bind_kr_dl_base_query(nb::module_& m);
void bind_kr_dl_ext_concept(nb::module_& m);
void bind_kr_dl_ext_role(nb::module_& m);
void bind_kr_dl_ext_boolean(nb::module_& m);
void bind_kr_dl_ext_numerical(nb::module_& m);
void bind_kr_dl_ext_constructor(nb::module_& m);
void bind_kr_dl_ext_query(nb::module_& m);
void bind_kr_dl_uns_concept(nb::module_& m);
void bind_kr_dl_uns_role(nb::module_& m);
void bind_kr_dl_uns_boolean(nb::module_& m);
void bind_kr_dl_uns_numerical(nb::module_& m);
void bind_kr_dl_uns_constructor(nb::module_& m);
void bind_kr_dl_uns_query(nb::module_& m);
void bind_kr_dl_denotation(nb::module_& m);
void bind_kr_ps_base(nb::module_& m);
void bind_kr_ps_ext(nb::module_& m);
void bind_kr_uns(nb::module_& m);
void bind_kr_execution(nb::module_& m);
void bind_graphs(nb::module_& m);

}  // namespace runir::serialization

#endif
