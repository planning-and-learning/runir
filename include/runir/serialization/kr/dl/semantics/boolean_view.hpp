#ifndef RUNIR_SERIALIZATION_KR_DL_SEMANTICS_BOOLEAN_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_DL_SEMANTICS_BOOLEAN_VIEW_HPP_

#include "runir/kr/dl/semantics/boolean_view.hpp"
#include "runir/serialization/kr/dl/argument_view.hpp"
#include "runir/serialization/kr/dl/semantics/constructor_view.hpp"

#include <tyr/serialization/formalism/predicate_view.hpp>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
struct TypeName<View<Index<runir::kr::dl::Boolean<Family, Tag>>, C>>
{
    static std::string get()
    {
        const auto prefix = std::string(Family::name) + "." + runir::kr::dl::BooleanTag::name + ".";
        if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
            return prefix + Tag::FactKind::name + "." + runir::kr::dl::BooleanAtomicStateSyntaxTag::keyword;
        else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
            return prefix + Tag::FactKind::name + "." + runir::kr::dl::BooleanAtomicGoalSyntaxTag::keyword;
        else
            return prefix + Tag::keyword;
    }
};

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::dl::Boolean<Family, Tag>>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        if constexpr (requires { value.get_predicate(); })
            ar.field("predicate", value.get_predicate());
        if constexpr (requires { value.get_polarity(); })
            ar.field("polarity", value.get_polarity());
        if constexpr (requires { value.get_argument(); })
            ar.field("argument", value.get_argument());
        if constexpr (requires { value.get_arg(); })
            ar.field("arg", value.get_arg());
        if constexpr (requires { value.get_lhs(); })
            ar.field("lhs", value.get_lhs());
        if constexpr (requires { value.get_rhs(); })
            ar.field("rhs", value.get_rhs());
        if constexpr (requires { value.get_value(); })
            ar.field("value", value.get_value());
    });
}

}

#endif

