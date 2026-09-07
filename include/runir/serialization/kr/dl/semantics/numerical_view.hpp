#ifndef RUNIR_SERIALIZATION_KR_DL_SEMANTICS_NUMERICAL_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_DL_SEMANTICS_NUMERICAL_VIEW_HPP_

#include "runir/kr/dl/semantics/numerical_view.hpp"
#include "runir/serialization/kr/dl/argument_view.hpp"
#include "runir/serialization/kr/dl/semantics/constructor_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
struct TypeName<View<Index<runir::kr::dl::Numerical<Family, Tag>>, C>>
{
    static std::string get()
    {
        const auto prefix = std::string(Family::name) + "." + runir::kr::dl::NumericalTag::name + ".";
        return prefix + Tag::keyword;
    }
};

template<typename Archive, runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::dl::Numerical<Family, Tag>>, C>>)
{
    using Value = View<Index<runir::kr::dl::Numerical<Family, Tag>>, C>;

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

}

#endif

