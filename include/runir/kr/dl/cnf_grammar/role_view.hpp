#ifndef RUNIR_CNF_GRAMMAR_ROLE_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_ROLE_VIEW_HPP_

#include "runir/kr/dl/cnf_grammar/role_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/formalism/predicate_view.hpp>

namespace tyr
{

template<runir::kr::dl::RoleConstructorTag Tag, typename C>
class View<Index<runir::kr::dl::cnf_grammar::Role<Tag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::dl::cnf_grammar::Role<Tag>> m_handle;

public:
    View(Index<runir::kr::dl::cnf_grammar::Role<Tag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }

    auto get_predicate() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    {
        return make_view(get_data().predicate, m_context->get_planning_repository());
    }

    auto get_polarity() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    {
        return get_data().polarity;
    }

    auto get_arg() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::ComplementTag> || std::same_as<Tag, runir::kr::dl::InverseTag>
                 || std::same_as<Tag, runir::kr::dl::TransitiveClosureTag> || std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>
                 || std::same_as<Tag, runir::kr::dl::IdentityTag>)
    {
        return make_view(get_data().arg, *m_context);
    }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                 || std::same_as<Tag, runir::kr::dl::CompositionTag> || std::same_as<Tag, runir::kr::dl::RestrictionTag>)
    {
        return make_view(get_data().lhs, *m_context);
    }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                 || std::same_as<Tag, runir::kr::dl::CompositionTag> || std::same_as<Tag, runir::kr::dl::RestrictionTag>)
    {
        return make_view(get_data().rhs, *m_context);
    }

    auto identifying_members() const noexcept { return std::tie(get_data()); }
};

}

#endif
