#include "module.hpp"

#include "bindings.hpp"

namespace runir::kr::dl::uns
{

void bind_semantics_module_definitions(nb::module_& m)
{
    bind_semantics_concept(m);
    bind_semantics_role(m);
    bind_semantics_boolean(m);
    bind_semantics_numerical(m);
    bind_semantics_constructor(m);
    bind_semantics_repositories(m);
}

}  // namespace runir::kr::dl::uns
