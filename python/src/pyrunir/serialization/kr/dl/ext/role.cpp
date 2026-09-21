#include "pyrunir/serialization/binding_utils.hpp"

namespace runir::serialization
{

void bind_kr_dl_ext_role(nb::module_& m) { bind_entity_types(m, DlComponentViews<kr::ExtFamilyTag, kr::dl::FamilyRoleTypes<kr::ExtFamilyTag>> {}); }

}  // namespace runir::serialization
