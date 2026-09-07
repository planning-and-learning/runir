#ifndef RUNIR_SERIALIZATION_KR_DL_SEMANTICS_ROLE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_DL_SEMANTICS_ROLE_VIEW_HPP_

#include "runir/kr/dl/semantics/role_view.hpp"
#include "runir/serialization/kr/dl/argument_view.hpp"
#include "runir/serialization/kr/dl/register_view.hpp"
#include "runir/serialization/kr/dl/semantics/constructor_view.hpp"

#include <tyr/serialization/formalism/predicate_view.hpp>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::dl::Role<Family, Tag>>, C>>)
{
    using Value = View<Index<runir::kr::dl::Role<Family, Tag>>, C>;

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

}

#endif

