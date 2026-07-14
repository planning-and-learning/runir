#include "module.hpp"

#include "semantics/module.hpp"

namespace runir::kr::dl::ext
{

void bind_module_definitions(nb::module_& m)
{
    bind_indices(m);
    bind_datas(m);
    bind_views(m);
    bind_parser(m);
    bind_repository(m);

    auto semantics_module = m.def_submodule("semantics");
    bind_semantics_module_definitions(semantics_module);
}

}  // namespace runir::kr::dl::ext
