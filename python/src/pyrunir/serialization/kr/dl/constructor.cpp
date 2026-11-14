#include "pyrunir/module.hpp"
#include "pyrunir/serialization/binding_utils.hpp"
#include "pyrunir/serialization/bindings.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/serialization/kr/dl/semantics/constructor_view.hpp>

namespace runir::serialization
{

void bind_kr_dl_constructor(nb::module_& m)
{
    using Types = boost::mp11::mp_append<kr::dl::ConstructorTypes, kr::dl::ReferenceTypes>;
    bind_entity_types(m, DlComponentViews<Types> {});
}

}  // namespace runir::serialization

namespace ygg::serialization
{

namespace
{

template<typename View>
void describe_variant(DictionaryWriter<View>& ar)
{
    ar.variant([](const auto& value) -> decltype(auto) { return (value.get_variant()); });
}

}  // namespace

void describe_fields(DictionaryWriter<runir::kr::dl::QueryView<>>& ar, std::type_identity<runir::kr::dl::QueryView<>> /*type*/) { describe_variant(ar); }
void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::ConceptTag>>& ar,
                     std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::ConceptTag>> /*type*/)
{ describe_variant(ar); }
void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::RoleTag>>& ar,
                     std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::RoleTag>> /*type*/)
{ describe_variant(ar); }
void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::BooleanTag>>& ar,
                     std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::BooleanTag>> /*type*/)
{ describe_variant(ar); }
void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::NumericalTag>>& ar,
                     std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::NumericalTag>> /*type*/)
{ describe_variant(ar); }

}  // namespace ygg::serialization
