#include "module.hpp"
#include "pyrunir/kr/dl/serialization.hpp"

#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/record_view.hpp>
#include <runir/kr/dl/repository.hpp>

namespace runir::kr::dl
{
void bind_query_serialization()
{
    using Types = boost::mp11::mp_append<runir::kr::dl::QueryTypes,
                                         boost::mp11::mp_list<runir::kr::dl::Query<>,
                                                              runir::kr::dl::QueryColumn,
                                                              runir::kr::dl::QueryProjection<runir::kr::dl::ConceptTag>,
                                                              runir::kr::dl::QueryProjection<runir::kr::dl::RoleTag>>>;
    ygg::python::bind_serialization(ygg::python::index_views<runir::kr::dl::ConstructorRepository, Types> {});
}
}  // namespace runir::kr::dl
