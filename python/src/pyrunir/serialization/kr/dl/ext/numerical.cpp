#include "pyrunir/serialization/binding_utils.hpp"

namespace runir::serialization
{

void bind_kr_dl_ext_numerical(nb::module_& m) { bind_entity_types(m, DlComponentViews<kr::ExtFamilyTag, kr::dl::FamilyNumericalTypes<kr::ExtFamilyTag>> {}); }

}  // namespace runir::serialization
