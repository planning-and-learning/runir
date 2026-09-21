#include "pyrunir/serialization/binding_utils.hpp"

namespace runir::serialization
{

void bind_kr_dl_ext_constructor(nb::module_& m)
{
    using Family = kr::ExtFamilyTag;
    using Types = ygg::ConcatTypeListsT<kr::dl::FamilyConstructorTypes<Family>, kr::dl::FamilyReferenceTypes<Family>>;
    bind_entity_types(m, DlComponentViews<Family, Types> {});
}

}  // namespace runir::serialization
