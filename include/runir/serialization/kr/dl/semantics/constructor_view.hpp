#ifndef RUNIR_SERIALIZATION_KR_DL_SEMANTICS_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_DL_SEMANTICS_CONSTRUCTOR_VIEW_HPP_

#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/record_view.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/serialization/kr/dl/argument_view.hpp"
#include "runir/serialization/kr/dl/register_view.hpp"

#include <tyr/serialization/formalism/planning/planning.hpp>
#include <yggdrasil/serialization/dictionaries.hpp>
#include <yggdrasil/serialization/fields.hpp>

/// Serialization of DL constructors of every category and of queries: every record by its fields
/// (yggdrasil/serialization/fields.hpp), a constructor or a query of any kind by its variant.

namespace ygg::serialization
{

/// A query of any kind: its variant.
template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<urd::index_view<runir::kr::dl::Query<>, C>> /*type*/)
{
    ar.variant([](const auto& value) -> decltype(auto) { return (value.get_variant()); });
}

template<typename Archive, runir::kr::dl::CategoryTag Category, typename C>
void describe_fields(Archive& ar, std::type_identity<urd::index_view<runir::kr::dl::Constructor<Category>, C>>)
{
    ar.variant([](const auto& value) -> decltype(auto) { return (value.get_variant()); });
}

/// Writing a constructor or a query of any kind writes the record it holds, and so every kind below it: written once,
/// in pyrunir (pyrunir/serialization/kr/dl/constructor.cpp), instead of in every file that writes a record holding one
/// (brokkr's template rules, rule 24).
template<typename T>
using DictionaryWriter = FieldWriter<Dictionaries::Archive, T>;
void describe_fields(DictionaryWriter<runir::kr::dl::QueryView<>>& ar, std::type_identity<runir::kr::dl::QueryView<>> type);
void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::ConceptTag>>& ar,
                     std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::ConceptTag>> type);
void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::RoleTag>>& ar,
                     std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::RoleTag>> type);
void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::BooleanTag>>& ar,
                     std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::BooleanTag>> type);
void describe_fields(DictionaryWriter<runir::kr::dl::ConstructorView<runir::kr::dl::NumericalTag>>& ar,
                     std::type_identity<runir::kr::dl::ConstructorView<runir::kr::dl::NumericalTag>> type);

}

#endif
