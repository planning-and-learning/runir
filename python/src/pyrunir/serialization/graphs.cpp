#include "pyrunir/serialization/binding_utils.hpp"

namespace runir::serialization
{

void bind_graphs(nb::module_& m)
{
    bind_entity_types(m, GraphProperties {});
    bind_serialized_types(m, Graphs {});
}

}  // namespace runir::serialization
