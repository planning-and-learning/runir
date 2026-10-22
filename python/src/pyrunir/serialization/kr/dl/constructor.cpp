#include "pyrunir/serialization/binding_utils.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/serialization/kr/dl/semantics/constructor_view.hpp>

namespace runir::serialization
{

void bind_kr_dl_constructor(nb::module_& m)
{
    using Types = ygg::ConcatTypeListsT<kr::dl::FamilyConstructorTypes, kr::dl::FamilyReferenceTypes>;
    bind_entity_types(m, DlComponentViews<Types> {});
}

}  // namespace runir::serialization
