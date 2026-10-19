#include "pyrunir/serialization/binding_utils.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/serialization/kr/dl/semantics/constructor_view.hpp>

namespace runir::serialization
{

void bind_kr_dl_base_numerical(nb::module_& m)
{
    bind_entity_types(m, DlComponentViews<kr::BaseFamilyTag, kr::dl::FamilyNumericalTypes<kr::BaseFamilyTag>> {});
}

}  // namespace runir::serialization
