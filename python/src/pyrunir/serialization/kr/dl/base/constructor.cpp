#include "pyrunir/serialization/binding_utils.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/serialization/kr/dl/semantics/constructor_view.hpp>

namespace runir::serialization
{

void bind_kr_dl_base_constructor(nb::module_& m)
{
    using Family = kr::BaseFamilyTag;
    using Types = ygg::ConcatTypeListsT<kr::dl::FamilyConstructorTypes<Family>, kr::dl::FamilyReferenceTypes<Family>>;
    bind_entity_types(m, DlComponentViews<Family, Types> {});
}

}  // namespace runir::serialization
