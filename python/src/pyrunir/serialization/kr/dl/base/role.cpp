#include "pyrunir/serialization/binding_utils.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/serialization/kr/dl/semantics/constructor_view.hpp>

namespace runir::serialization
{

void bind_kr_dl_base_role(nb::module_& m) { bind_entity_types(m, DlComponentViews<kr::BaseFamilyTag, kr::dl::FamilyRoleTypes<kr::BaseFamilyTag>> {}); }

}  // namespace runir::serialization
