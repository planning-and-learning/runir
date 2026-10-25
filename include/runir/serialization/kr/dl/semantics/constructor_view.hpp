#ifndef RUNIR_SERIALIZATION_KR_DL_SEMANTICS_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_DL_SEMANTICS_CONSTRUCTOR_VIEW_HPP_

#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/boolean_view.hpp"
#include "runir/kr/dl/semantics/concept_view.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/dl/semantics/numerical_view.hpp"
#include "runir/kr/dl/semantics/role_view.hpp"
#include "runir/serialization/kr/dl/argument_view.hpp"
#include "runir/serialization/kr/dl/register_view.hpp"

#include <tyr/serialization/formalism/object_view.hpp>
#include <tyr/serialization/formalism/predicate_view.hpp>
#include <yggdrasil/serialization/dictionaries.hpp>

/// Serialization of DL constructors of every category and of queries: their fields refer to each other, so they are
/// described together.

namespace ygg::serialization
{

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::dl::QueryColumn>, C>>)
{
    ar.field("name", [](const auto& value) -> decltype(auto) { return (value.get_name()); });
}

template<typename Archive, typename Tag, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::dl::Query<Tag>>, C>>)
{
    using Value = View<Index<runir::kr::dl::Query<Tag>>, C>;

    if constexpr (std::same_as<Tag, void>)
        ar.variant([](const auto& value) -> decltype(auto) { return (value.get_variant()); });
    else
    {
        if constexpr (requires(const Value& value) { value.get_predicate(); })
            ar.field("predicate", [](const auto& value) -> decltype(auto) { return (value.get_predicate()); });
        if constexpr (requires(const Value& value) { value.get_polarity(); })
            ar.field("polarity", [](const auto& value) -> decltype(auto) { return (value.get_polarity()); });
        ar.field("columns", [](const auto& value) -> decltype(auto) { return (value.get_columns()); });
        if constexpr (requires(const Value& value) { value.get_arg(); })
            ar.field("arg", [](const auto& value) -> decltype(auto) { return (value.get_arg()); });
        if constexpr (requires(const Value& value) { value.get_args(); })
            ar.field("args", [](const auto& value) -> decltype(auto) { return (value.get_args()); });
        if constexpr (requires(const Value& value) { value.get_variable_order(); })
            ar.field("variable_order", [](const auto& value) -> decltype(auto) { return (value.get_variable_order()); });
        if constexpr (requires(const Value& value) { value.get_output_order(); })
            ar.field("output_order", [](const auto& value) -> decltype(auto) { return (value.get_output_order()); });
        if constexpr (requires(const Value& value) { value.get_lhs(); })
            ar.field("lhs", [](const auto& value) -> decltype(auto) { return (value.get_lhs()); });
        if constexpr (requires(const Value& value) { value.get_rhs(); })
            ar.field("rhs", [](const auto& value) -> decltype(auto) { return (value.get_rhs()); });
        if constexpr (requires(const Value& value) { value.get_lhs_column(); })
            ar.field("lhs_column", [](const auto& value) -> decltype(auto) { return (value.get_lhs_column()); });
        if constexpr (requires(const Value& value) { value.get_rhs_column(); })
            ar.field("rhs_column", [](const auto& value) -> decltype(auto) { return (value.get_rhs_column()); });
        if constexpr (requires(const Value& value) { value.get_column(); })
            ar.field("column", [](const auto& value) -> decltype(auto) { return (value.get_column()); });
        if constexpr (requires(const Value& value) { value.get_object(); })
            ar.field("object", [](const auto& value) -> decltype(auto) { return (value.get_object()); });
    }
}

template<typename Archive, runir::kr::dl::ConceptOrRoleTag Category, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::dl::QueryProjection<Category>>, C>>)
{
    ar.field("columns", [](const auto& value) -> decltype(auto) { return (value.get_columns()); });
    ar.field("arg", [](const auto& value) -> decltype(auto) { return (value.get_arg()); });
}

template<typename Archive, typename Tag, typename C>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::dl::Boolean<Tag>>, C>>)
{
    using Value = View<Index<runir::kr::dl::Boolean<Tag>>, C>;

    if constexpr (requires(const Value& value) { value.get_predicate(); })
        ar.field("predicate", [](const auto& value) -> decltype(auto) { return (value.get_predicate()); });
    if constexpr (requires(const Value& value) { value.get_polarity(); })
        ar.field("polarity", [](const auto& value) -> decltype(auto) { return (value.get_polarity()); });
    if constexpr (requires(const Value& value) { value.get_argument(); })
        ar.field("argument", [](const auto& value) -> decltype(auto) { return (value.get_argument()); });
    if constexpr (requires(const Value& value) { value.get_arg(); })
        ar.field("arg", [](const auto& value) -> decltype(auto) { return (value.get_arg()); });
    if constexpr (requires(const Value& value) { value.get_lhs(); })
        ar.field("lhs", [](const auto& value) -> decltype(auto) { return (value.get_lhs()); });
    if constexpr (requires(const Value& value) { value.get_rhs(); })
        ar.field("rhs", [](const auto& value) -> decltype(auto) { return (value.get_rhs()); });
    if constexpr (requires(const Value& value) { value.get_value(); })
        ar.field("value", [](const auto& value) -> decltype(auto) { return (value.get_value()); });
}

template<typename Archive, typename Tag, typename C>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::dl::Concept<Tag>>, C>>)
{
    using Value = View<Index<runir::kr::dl::Concept<Tag>>, C>;

    if constexpr (requires(const Value& value) { value.get_predicate(); })
        ar.field("predicate", [](const auto& value) -> decltype(auto) { return (value.get_predicate()); });
    if constexpr (requires(const Value& value) { value.get_polarity(); })
        ar.field("polarity", [](const auto& value) -> decltype(auto) { return (value.get_polarity()); });
    if constexpr (requires(const Value& value) { value.get_object(); })
        ar.field("object", [](const auto& value) -> decltype(auto) { return (value.get_object()); });
    if constexpr (requires(const Value& value) { value.get_register(); })
        ar.field("register", [](const auto& value) -> decltype(auto) { return (value.get_register()); });
    if constexpr (requires(const Value& value) { value.get_argument(); })
        ar.field("argument", [](const auto& value) -> decltype(auto) { return (value.get_argument()); });
    if constexpr (requires(const Value& value) { value.get_objects(); })
        ar.field("objects", [](const auto& value) -> decltype(auto) { return (value.get_objects()); });
    if constexpr (requires(const Value& value) { value.get_n(); })
        ar.field("n", [](const auto& value) -> decltype(auto) { return (value.get_n()); });
    if constexpr (requires(const Value& value) { value.get_role(); })
        ar.field("role", [](const auto& value) -> decltype(auto) { return (value.get_role()); });
    if constexpr (requires(const Value& value) { value.get_concept(); })
        ar.field("concept", [](const auto& value) -> decltype(auto) { return (value.get_concept()); });
    if constexpr (requires(const Value& value) { value.get_arg(); })
        ar.field("arg", [](const auto& value) -> decltype(auto) { return (value.get_arg()); });
    if constexpr (requires(const Value& value) { value.get_lhs(); })
        ar.field("lhs", [](const auto& value) -> decltype(auto) { return (value.get_lhs()); });
    if constexpr (requires(const Value& value) { value.get_rhs(); })
        ar.field("rhs", [](const auto& value) -> decltype(auto) { return (value.get_rhs()); });
}

template<typename Archive, typename Tag, typename C>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::dl::Numerical<Tag>>, C>>)
{
    using Value = View<Index<runir::kr::dl::Numerical<Tag>>, C>;

    if constexpr (requires(const Value& value) { value.get_argument(); })
        ar.field("argument", [](const auto& value) -> decltype(auto) { return (value.get_argument()); });
    if constexpr (requires(const Value& value) { value.get_arg(); })
        ar.field("arg", [](const auto& value) -> decltype(auto) { return (value.get_arg()); });
    if constexpr (requires(const Value& value) { value.get_lhs(); })
        ar.field("lhs", [](const auto& value) -> decltype(auto) { return (value.get_lhs()); });
    if constexpr (requires(const Value& value) { value.get_mid(); })
        ar.field("mid", [](const auto& value) -> decltype(auto) { return (value.get_mid()); });
    if constexpr (requires(const Value& value) { value.get_rhs(); })
        ar.field("rhs", [](const auto& value) -> decltype(auto) { return (value.get_rhs()); });
    if constexpr (requires(const Value& value) { value.get_value(); })
        ar.field("value", [](const auto& value) -> decltype(auto) { return (value.get_value()); });
}

template<typename Archive, typename Tag, typename C>
    requires runir::kr::dl::RoleConstructorTag<Tag>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::dl::Role<Tag>>, C>>)
{
    using Value = View<Index<runir::kr::dl::Role<Tag>>, C>;

    if constexpr (requires(const Value& value) { value.get_predicate(); })
        ar.field("predicate", [](const auto& value) -> decltype(auto) { return (value.get_predicate()); });
    if constexpr (requires(const Value& value) { value.get_polarity(); })
        ar.field("polarity", [](const auto& value) -> decltype(auto) { return (value.get_polarity()); });
    if constexpr (requires(const Value& value) { value.get_register(); })
        ar.field("register", [](const auto& value) -> decltype(auto) { return (value.get_register()); });
    if constexpr (requires(const Value& value) { value.get_argument(); })
        ar.field("argument", [](const auto& value) -> decltype(auto) { return (value.get_argument()); });
    if constexpr (requires(const Value& value) { value.get_arg(); })
        ar.field("arg", [](const auto& value) -> decltype(auto) { return (value.get_arg()); });
    if constexpr (requires(const Value& value) { value.get_lhs(); })
        ar.field("lhs", [](const auto& value) -> decltype(auto) { return (value.get_lhs()); });
    if constexpr (requires(const Value& value) { value.get_rhs(); })
        ar.field("rhs", [](const auto& value) -> decltype(auto) { return (value.get_rhs()); });
}

template<typename Archive, runir::kr::dl::CategoryTag Category, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::dl::Constructor<Category>>, C>>)
{
    ar.variant([](const auto& value) -> decltype(auto) { return (value.get_variant()); });
}

/// Variant operands of constructors (concept, role or query), e.g. count's argument or distance's
/// vertices and edges, serialize as the alternative they hold.
template<typename Archive, typename... Ts, typename C>
void describe_fields(Archive& ar, std::type_identity<View<cista::offset::variant<Index<Ts>...>, C>>)
{
    ar.variant([](const auto& value) -> decltype(auto) { return (value); });
}

}

#endif
