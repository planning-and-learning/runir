#ifndef RUNIR_SEMANTICS_ROLE_VIEW_HPP_
#define RUNIR_SEMANTICS_ROLE_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/dl/role_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/predicate_view.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename Tag, formalism::SymbolContextFor<runir::kr::dl::FamilyRole<Family, Tag>> C>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
class View<Index<runir::kr::dl::FamilyRole<Family, Tag>>, C> : public formalism::detail::View<Index<runir::kr::dl::FamilyRole<Family, Tag>>, C>
{
public:
    View(Index<runir::kr::dl::FamilyRole<Family, Tag>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::dl::FamilyRole<Family, Tag>>, C>(handle, context)
    {
    }

    auto get_predicate() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    {
        return make_view(this->get_data().predicate, this->m_context->get_planning_repository());
    }

    auto get_polarity() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    {
        return this->get_data().polarity;
    }

    auto get_register() const noexcept
        requires std::same_as<Tag, runir::kr::dl::RegisterTag>
    {
        return make_view(this->get_data().reference, *this->m_context);
    }

    auto get_argument() const noexcept
        requires std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>
    {
        return make_view(this->get_data().reference, *this->m_context);
    }

    auto get_arg() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::ComplementTag> || std::same_as<Tag, runir::kr::dl::InverseTag>
                 || std::same_as<Tag, runir::kr::dl::TransitiveClosureTag> || std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>
                 || std::same_as<Tag, runir::kr::dl::IdentityTag>)
    {
        return make_view(this->get_data().arg, *this->m_context);
    }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                 || std::same_as<Tag, runir::kr::dl::CompositionTag> || std::same_as<Tag, runir::kr::dl::RestrictionTag>)
    {
        return make_view(this->get_data().lhs, *this->m_context);
    }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                 || std::same_as<Tag, runir::kr::dl::CompositionTag> || std::same_as<Tag, runir::kr::dl::RestrictionTag>)
    {
        return make_view(this->get_data().rhs, *this->m_context);
    }
};

}

#endif
