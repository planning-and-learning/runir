#include "module.hpp"

#include <runir/serialization/serialization.hpp>
#include <runir/serialization/types.hpp>
#include <yggdrasil/python/serialization.hpp>

namespace runir::serialization
{
void bind_module_definitions(nb::module_& m)
{
    ygg::python::bind_serialization(m, RegisteredTypes {}, SerializedTypes {}, ProjectionTypes {});
}

}  // namespace runir::serialization
