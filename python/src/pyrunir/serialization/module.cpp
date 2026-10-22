#include "bindings.hpp"

namespace runir::serialization
{

void bind_module_definitions(nb::module_& m)
{
    bind_kr_dl_concept(m);
    bind_kr_dl_role(m);
    bind_kr_dl_boolean(m);
    bind_kr_dl_numerical(m);
    bind_kr_dl_constructor(m);
    bind_kr_dl_query(m);
    bind_kr_dl_denotation(m);
    bind_kr_ps_base(m);
    bind_kr_ps_ext(m);
    bind_kr_ps_icp(m);
    bind_kr_uns(m);
    bind_kr_execution(m);
    bind_graphs(m);
}

}  // namespace runir::serialization
