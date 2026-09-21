#include "pyrunir/serialization/binding_utils.hpp"

namespace runir::serialization
{

void bind_kr_dl_uns_concept(nb::module_& m) { bind_entity_types(m, DlComponentViews<kr::UnsFamilyTag, kr::dl::FamilyConceptTypes<kr::UnsFamilyTag>> {}); }

}  // namespace runir::serialization
