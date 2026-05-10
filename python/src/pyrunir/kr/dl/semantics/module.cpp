#include "module.hpp"

namespace runir::kr::dl
{

void bind_semantics_module_definitions(nb::module_& m)
{
    bind_semantics_indices(m);
    bind_semantics_datas(m);
    bind_semantics_views(m);
    bind_semantics_repositories(m);
}

}  // namespace runir::kr::dl
