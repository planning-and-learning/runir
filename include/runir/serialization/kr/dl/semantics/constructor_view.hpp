#ifndef RUNIR_SERIALIZATION_KR_DL_SEMANTICS_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_DL_SEMANTICS_CONSTRUCTOR_VIEW_HPP_

#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/record_view.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/serialization/kr/dl/argument_view.hpp"
#include "runir/serialization/kr/dl/register_view.hpp"

#include <concepts>
#include <type_traits>
#include <tyr/serialization/formalism/planning/planning.hpp>
#include <urd/elements/concepts.hpp>
#include <yggdrasil/serialization/dictionaries.hpp>
#include <yggdrasil/serialization/fields.hpp>

/// Serialization of DL constructors of every category and of queries: every record by its fields
/// (yggdrasil/serialization/fields.hpp), a record with derived fields by those that identify it, a constructor or a query
/// of any kind by its variant.

namespace ygg::serialization
{

/// A query of any kind: its variant.
template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<urd::index_view<runir::kr::dl::Query<>, C>> /*type*/)
{
    ar.variant([](const auto& value) -> decltype(auto) { return (value.get_variant()); });
}

/// Whether Member is one of the members that identify the record Data (its identifying_members()).
template<typename Data, auto Member>
inline constexpr bool identifies = []<auto... Ms>(urd::Members<Ms...> /*members*/)
{ return (std::is_same_v<std::integral_constant<decltype(Ms), Ms>, std::integral_constant<decltype(Member), Member>> || ...); }(Data::identifying_members());

/// A record with derived fields (a query's schema or its columns and plan, a distance's plan), which its repository
/// computes when it interns it: by the fields that identify it, in order.
template<typename Archive, typename T, typename C>
void describe_identifying_fields(Archive& ar, std::type_identity<urd::index_view<T, C>> /*type*/)
{
    urd::for_each_field<urd::data<T>>(
        [&]<typename F>()
        {
            if constexpr (identifies<urd::data<T>, F::member>)
                ar.field(F::name, [](const auto& value) -> decltype(auto) { return value.template member<F::member>(); });
        });
}

template<typename Archive, typename Tag, typename C>
    requires(!std::same_as<Tag, void>)
void describe_fields(Archive& ar, std::type_identity<urd::index_view<runir::kr::dl::Query<Tag>, C>> type)
{ describe_identifying_fields(ar, type); }

template<typename Archive, runir::kr::dl::ConceptOrRoleTag Category, typename C>
void describe_fields(Archive& ar, std::type_identity<urd::index_view<runir::kr::dl::QueryProjection<Category>, C>> type)
{ describe_identifying_fields(ar, type); }

template<typename Archive, typename C>
void describe_fields(Archive& ar,
                     std::type_identity<urd::index_view<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::DistanceTag>, C>> type)
{ describe_identifying_fields(ar, type); }

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
