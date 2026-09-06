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

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
struct TypeName<View<Index<runir::kr::dl::Role<Family, Tag>>, C>>
{
    static std::string get()
    {
        const auto prefix = std::string(Family::name) + "." + runir::kr::dl::RoleTag::name + ".";
        if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
            return prefix + Tag::FactKind::name + "." + runir::kr::dl::RoleAtomicStateSyntaxTag::keyword;
        else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
            return prefix + Tag::FactKind::name + "." + runir::kr::dl::RoleAtomicGoalSyntaxTag::keyword;
        else if constexpr (std::same_as<Tag, runir::kr::dl::RegisterTag>)
            return prefix + runir::kr::dl::RoleRegisterSyntaxTag::keyword;
        else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
            return prefix + runir::kr::dl::RoleIntersectionSyntaxTag::keyword;
        else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
            return prefix + runir::kr::dl::RoleUnionSyntaxTag::keyword;
        else
            return prefix + Tag::keyword;
    }
};

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::dl::Role<Family, Tag>>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        if constexpr (requires { value.get_predicate(); })
            ar.field("predicate", value.get_predicate());
        if constexpr (requires { value.get_polarity(); })
            ar.field("polarity", value.get_polarity());
        if constexpr (requires { value.get_register(); })
            ar.field("register", value.get_register());
        if constexpr (requires { value.get_argument(); })
            ar.field("argument", value.get_argument());
        if constexpr (requires { value.get_arg(); })
            ar.field("arg", value.get_arg());
        if constexpr (requires { value.get_lhs(); })
            ar.field("lhs", value.get_lhs());
        if constexpr (requires { value.get_rhs(); })
            ar.field("rhs", value.get_rhs());
    });
}

}

#endif

