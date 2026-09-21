#include "pyrunir/serialization/binding_utils.hpp"

namespace runir::serialization
{

void bind_kr_dl_base_role(nb::module_& m) { bind_entity_types(m, DlComponentViews<kr::BaseFamilyTag, kr::dl::FamilyRoleTypes<kr::BaseFamilyTag>> {}); }

}  // namespace runir::serialization
