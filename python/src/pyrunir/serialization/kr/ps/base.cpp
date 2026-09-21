#include "pyrunir/serialization/binding_utils.hpp"

namespace runir::serialization
{

void bind_kr_ps_base(nb::module_& m) { bind_entity_types(m, BaseViews {}); }

}  // namespace runir::serialization
