#include "module.hpp"

#include "bindings.hpp"
#include "semantics/module.hpp"

namespace runir::kr::dl::ext
{

void bind_module_definitions(nb::module_& m)
{
    bind_argument(m);
    bind_register(m);
    bind_constructor(m);
    bind_parser(m);

    auto semantics_module = m.def_submodule("semantics");
    bind_semantics_module_definitions(semantics_module);
    m.attr("ConstructorRepository") = semantics_module.attr("ConstructorRepository");
    m.attr("ConstructorRepositoryFactory") = semantics_module.attr("ConstructorRepositoryFactory");
}

}  // namespace runir::kr::dl::ext
