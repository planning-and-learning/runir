#ifndef PYRUNIR_KR_DL_SERIALIZATION_HPP_
#define PYRUNIR_KR_DL_SERIALIZATION_HPP_

#include <boost/mp11.hpp>
#include <runir/kr/dl/constructor_data.hpp>
#include <runir/kr/dl/declarations.hpp>
#include <runir/kr/dl/query_data.hpp>
#include <type_traits>
#include <urd/elements/view.hpp>
#include <yggdrasil/python/serialization.hpp>
#include <yggdrasil/serialization/dictionaries.hpp>

/// Writing a constructor or a query of any kind writes the record it holds, and so every kind below it: instantiated once,
/// in kr/dl/serialization_constructor.cpp, instead of in every source that writes a record holding one (brokkr's template
/// rules, rule 24). The generic describe_fields of a record's view (yggdrasil/serialization/fields.hpp), extern here.
namespace ygg::serialization
{
template<typename T>
using DictionaryWriter = FieldWriter<Dictionaries::Archive, T>;

extern template void describe_fields(DictionaryWriter<runir::kr::dl::QueryView<>>& ar, std::type_identity<runir::kr::dl::QueryView<>> type);
extern template void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::ConceptTag>>& ar,
                                     std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::ConceptTag>> type);
extern template void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::RoleTag>>& ar,
                                     std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::RoleTag>> type);
extern template void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::BooleanTag>>& ar,
                                     std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::BooleanTag>> type);
extern template void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::NumericalTag>>& ar,
                                     std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::NumericalTag>> type);
}  // namespace ygg::serialization

#endif
