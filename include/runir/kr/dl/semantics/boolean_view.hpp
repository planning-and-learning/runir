#ifndef RUNIR_SEMANTICS_BOOLEAN_VIEW_HPP_
#define RUNIR_SEMANTICS_BOOLEAN_VIEW_HPP_

#include "runir/kr/dl/boolean_data.hpp"
#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/uns/declarations.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/predicate_view.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
class View<Index<runir::kr::dl::FamilyBoolean<Family, Tag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::dl::FamilyBoolean<Family, Tag>> m_handle;

public:
    View(Index<runir::kr::dl::FamilyBoolean<Family, Tag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

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
        requires std::same_as<Tag, runir::kr::dl::NonemptyTag>
    {
        return make_view(get_data().arg, *m_context);
    }

    auto get_lhs() const noexcept
        requires runir::kr::dl::ComparisonTag<Tag>
    {
        return make_view(get_data().lhs, *m_context);
    }

    auto get_rhs() const noexcept
        requires runir::kr::dl::ComparisonTag<Tag>
    {
        return make_view(get_data().rhs, *m_context);
    }

    auto get_value() const noexcept
        requires std::same_as<Tag, runir::kr::dl::BooleanConstantTag>
    {
        return get_data().identifier;
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}

#endif
