#include "module.hpp"

namespace runir::datasets
{

void bind_module_definitions(nb::module_& m)
{
    bind_config(m);
    bind_task_class(m);
    bind_state_graph(m);
    bind_equivalence_graph(m);
}

}  // namespace runir::datasets
