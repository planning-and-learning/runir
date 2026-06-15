#include "module.hpp"

#include "semantics/module.hpp"

namespace runir::kr::dl::uns
{

void bind_module_definitions(nb::module_& m)
{
    bind_indices(m);
    bind_parser(m);
    bind_repository(m);

    auto semantics_module = m.def_submodule("semantics");
    bind_semantics_module_definitions(semantics_module);
}

}  // namespace runir::kr::dl::uns
