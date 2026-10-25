#include "pyrunir/serialization/binding_utils.hpp"
#include "pyrunir/serialization/bindings.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/serialization/kr/dl/semantics/constructor_view.hpp>

namespace runir::serialization
{

void bind_kr_dl_query(nb::module_& m)
{
    using Types = ygg::ConcatTypeListsT<
        kr::dl::QueryTypes,
        ygg::TypeList<kr::dl::Query<>, kr::dl::QueryColumn, kr::dl::QueryProjection<kr::dl::ConceptTag>, kr::dl::QueryProjection<kr::dl::RoleTag>>>;
    bind_entity_types(m, DlComponentViews<Types> {});
}

}  // namespace runir::serialization
