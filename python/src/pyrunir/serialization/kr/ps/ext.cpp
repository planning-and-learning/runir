#include "pyrunir/serialization/binding_utils.hpp"

namespace runir::serialization
{

void bind_kr_ps_ext(nb::module_& m) { bind_entity_types(m, ExtViews {}); }

}  // namespace runir::serialization
