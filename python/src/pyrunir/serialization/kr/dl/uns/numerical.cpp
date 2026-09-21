#include "pyrunir/serialization/binding_utils.hpp"

namespace runir::serialization
{

void bind_kr_dl_uns_numerical(nb::module_& m) { bind_entity_types(m, DlComponentViews<kr::UnsFamilyTag, kr::dl::FamilyNumericalTypes<kr::UnsFamilyTag>> {}); }

}  // namespace runir::serialization
