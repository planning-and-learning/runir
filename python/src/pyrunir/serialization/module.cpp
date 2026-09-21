#include "bindings.hpp"

namespace runir::serialization
{

void bind_module_definitions(nb::module_& m)
{
    bind_kr_dl_base_concept(m);
    bind_kr_dl_base_role(m);
    bind_kr_dl_base_boolean(m);
    bind_kr_dl_base_numerical(m);
    bind_kr_dl_base_constructor(m);
    bind_kr_dl_base_query(m);
    bind_kr_dl_ext_concept(m);
    bind_kr_dl_ext_role(m);
    bind_kr_dl_ext_boolean(m);
    bind_kr_dl_ext_numerical(m);
    bind_kr_dl_ext_constructor(m);
    bind_kr_dl_ext_query(m);
    bind_kr_dl_uns_concept(m);
    bind_kr_dl_uns_role(m);
    bind_kr_dl_uns_boolean(m);
    bind_kr_dl_uns_numerical(m);
    bind_kr_dl_uns_constructor(m);
    bind_kr_dl_uns_query(m);
    bind_kr_dl_denotation(m);
    bind_kr_ps_base(m);
    bind_kr_ps_ext(m);
    bind_kr_uns(m);
    bind_kr_execution(m);
    bind_graphs(m);
}

}  // namespace runir::serialization
