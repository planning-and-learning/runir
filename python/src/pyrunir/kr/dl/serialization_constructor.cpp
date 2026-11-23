#include "module.hpp"
#include "pyrunir/kr/dl/serialization.hpp"

#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/record_view.hpp>
#include <runir/kr/dl/repository.hpp>

namespace runir::kr::dl
{
void bind_constructor_serialization()
{
    using Types = boost::mp11::mp_append<runir::kr::dl::ConstructorTypes, runir::kr::dl::ReferenceTypes>;
    ygg::python::bind_serialization(ygg::python::index_views<runir::kr::dl::ConstructorRepository, Types> {});
}
}  // namespace runir::kr::dl

namespace ygg::serialization
{
template void describe_fields(DictionaryWriter<runir::kr::dl::QueryView<>>& ar, std::type_identity<runir::kr::dl::QueryView<>> type);
template void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::ConceptTag>>& ar,
                              std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::ConceptTag>> type);
template void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::RoleTag>>& ar,
                              std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::RoleTag>> type);
template void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::BooleanTag>>& ar,
                              std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::BooleanTag>> type);
template void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::NumericalTag>>& ar,
                              std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::NumericalTag>> type);
}  // namespace ygg::serialization
