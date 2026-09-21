#include "pyrunir/serialization/binding_utils.hpp"

namespace runir::serialization
{

void bind_kr_dl_denotation(nb::module_& m) { bind_entity_types(m, DenotationViews {}); }

}  // namespace runir::serialization
