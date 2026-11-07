#include "module.hpp"
#include "pyrunir/module.hpp"

#include "bindings.hpp"

namespace runir::kr::dl
{

void bind_semantics_module_definitions(nb::module_& m)
{
    bind_semantics_denotation(m);
    bind_semantics_query(m);
    bind_semantics_records(m);
    bind_semantics_constructor(m);
    bind_semantics_repositories(m);
}

}  // namespace runir::kr::dl
