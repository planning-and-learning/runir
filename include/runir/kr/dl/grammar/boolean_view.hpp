#ifndef RUNIR_GRAMMAR_BOOLEAN_VIEW_HPP_
#define RUNIR_GRAMMAR_BOOLEAN_VIEW_HPP_

#include "runir/kr/dl/grammar/boolean_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/predicate_view.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename Tag, formalism::SymbolContextFor<runir::kr::dl::grammar::Boolean<Family, Tag>> C>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
class View<Index<runir::kr::dl::grammar::Boolean<Family, Tag>>, C> : public formalism::detail::View<Index<runir::kr::dl::grammar::Boolean<Family, Tag>>, C>
{
public:
    View(Index<runir::kr::dl::grammar::Boolean<Family, Tag>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::dl::grammar::Boolean<Family, Tag>>, C>(handle, context)
    {
    }

    auto get_identifier() const noexcept
        requires std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>
    {
        return this->get_data().identifier;
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

    auto get_arg() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::NonemptyTag> || std::same_as<Tag, runir::kr::dl::NotTag>)
    {
        return make_view(this->get_data().arg, *this->m_context);
    }

    auto get_lhs() const noexcept
        requires(runir::kr::dl::ComparisonTag<Tag> || runir::kr::dl::LogicalBinaryTag<Tag>)
    {
        return make_view(this->get_data().lhs, *this->m_context);
    }

    auto get_rhs() const noexcept
        requires(runir::kr::dl::ComparisonTag<Tag> || runir::kr::dl::LogicalBinaryTag<Tag>)
    {
        return make_view(this->get_data().rhs, *this->m_context);
    }

    auto get_value() const noexcept
        requires std::same_as<Tag, runir::kr::dl::BooleanConstantTag>
    {
        return this->get_data().identifier;
    }
};

}

#endif
