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

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::dl::Numerical<Family, Tag>>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        if constexpr (requires { value.get_argument(); })
            ar.field("argument", value.get_argument());
        if constexpr (requires { value.get_arg(); })
            ar.field("arg", value.get_arg());
        if constexpr (requires { value.get_lhs(); })
            ar.field("lhs", value.get_lhs());
        if constexpr (requires { value.get_mid(); })
            ar.field("mid", value.get_mid());
        if constexpr (requires { value.get_rhs(); })
            ar.field("rhs", value.get_rhs());
        if constexpr (requires { value.get_value(); })
            ar.field("value", value.get_value());
    });
}

}

#endif

