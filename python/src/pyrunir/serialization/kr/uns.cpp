#include "pyrunir/serialization/binding_utils.hpp"

namespace runir::serialization
{

void bind_kr_uns(nb::module_& m) { bind_entity_types(m, UnsViews {}); }

}  // namespace runir::serialization
