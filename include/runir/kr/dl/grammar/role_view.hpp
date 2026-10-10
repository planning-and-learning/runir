#ifndef RUNIR_GRAMMAR_ROLE_VIEW_HPP_
#define RUNIR_GRAMMAR_ROLE_VIEW_HPP_

#include "runir/kr/dl/grammar/role_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/predicate_view.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename Tag, formalism::SymbolContextFor<runir::kr::dl::grammar::Role<Family, Tag>> C>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
class View<Index<runir::kr::dl::grammar::Role<Family, Tag>>, C> : public ygg::IndexViewBase<runir::kr::dl::grammar::Role<Family, Tag>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::grammar::Role<Family, Tag>, C>::IndexViewBase;

    auto get_identifier() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::RegisterTag> || std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>)
    {
        return this->get_data().identifier;
    }

    auto get_predicate() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    {
        return make_view(this->get_data().predicate, this->get_context().get_planning_repository());
    }

    auto get_polarity() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    {
        return this->get_data().polarity;
    }

    auto get_arg() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::ComplementTag> || std::same_as<Tag, runir::kr::dl::InverseTag>
                 || std::same_as<Tag, runir::kr::dl::TransitiveClosureTag> || std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>
                 || std::same_as<Tag, runir::kr::dl::IdentityTag>)
    {
        return make_view(this->get_data().arg, this->get_context());
    }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                 || std::same_as<Tag, runir::kr::dl::CompositionTag> || std::same_as<Tag, runir::kr::dl::RestrictionTag>)
    {
        return make_view(this->get_data().lhs, this->get_context());
    }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                 || std::same_as<Tag, runir::kr::dl::CompositionTag> || std::same_as<Tag, runir::kr::dl::RestrictionTag>)
    {
        return make_view(this->get_data().rhs, this->get_context());
    }
};

}

#endif
