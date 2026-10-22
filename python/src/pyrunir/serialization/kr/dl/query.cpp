#include "pyrunir/serialization/binding_utils.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/serialization/kr/dl/semantics/constructor_view.hpp>

namespace runir::serialization
{

void bind_kr_dl_query(nb::module_& m)
{
    using Family = kr::ExtFamilyTag;
    using Types = ygg::ConcatTypeListsT<kr::dl::FamilyQueryTypes<Family>,
                                        ygg::TypeList<kr::dl::Query<Family>,
                                                      kr::dl::QueryColumn,
                                                      kr::dl::QueryProjection<Family, kr::dl::ConceptTag>,
                                                      kr::dl::QueryProjection<Family, kr::dl::RoleTag>>>;
    bind_entity_types(m, DlComponentViews<Family, Types> {});
}

}  // namespace runir::serialization
