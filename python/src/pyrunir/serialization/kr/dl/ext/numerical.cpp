#include "pyrunir/serialization/binding_utils.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/serialization/kr/dl/semantics/constructor_view.hpp>

namespace runir::serialization
{

void bind_kr_dl_ext_numerical(nb::module_& m) { bind_entity_types(m, DlComponentViews<kr::ExtFamilyTag, kr::dl::FamilyNumericalTypes<kr::ExtFamilyTag>> {}); }

}  // namespace runir::serialization
