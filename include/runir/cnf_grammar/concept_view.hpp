#ifndef RUNIR_CNF_GRAMMAR_CONCEPT_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_CONCEPT_VIEW_HPP_

#include "runir/cnf_grammar/concept_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/predicate_view.hpp>

namespace tyr
{

template<runir::ConceptConstructorTag Tag, typename C>
class View<Index<runir::cnf_grammar::Concept<Tag>>, C>
{
private:
    const C* m_context;
    Index<runir::cnf_grammar::Concept<Tag>> m_handle;

public:
    View(Index<runir::cnf_grammar::Concept<Tag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }

    auto get_predicate() const noexcept
        requires(runir::is_atomic_state_tag_v<Tag> || runir::is_atomic_goal_tag_v<Tag>)
    {
        return make_view(get_data().predicate, *m_context);
    }

    auto get_polarity() const noexcept
        requires(runir::is_atomic_state_tag_v<Tag> || runir::is_atomic_goal_tag_v<Tag>)
    {
        return get_data().polarity;
    }

    auto get_object() const noexcept
        requires std::same_as<Tag, runir::NominalTag>
    {
        return make_view(get_data().object, *m_context);
    }

    auto get_arg() const noexcept
        requires std::same_as<Tag, runir::NegationTag>
    {
        return make_view(get_data().arg, *m_context);
    }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::IntersectionTag> || std::same_as<Tag, runir::UnionTag> || std::same_as<Tag, runir::ValueRestrictionTag>
                 || std::same_as<Tag, runir::ExistentialQuantificationTag> || std::same_as<Tag, runir::RoleValueMapContainmentTag>
                 || std::same_as<Tag, runir::RoleValueMapEqualityTag>)
    {
        return make_view(get_data().lhs, *m_context);
    }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::IntersectionTag> || std::same_as<Tag, runir::UnionTag> || std::same_as<Tag, runir::ValueRestrictionTag>
                 || std::same_as<Tag, runir::ExistentialQuantificationTag> || std::same_as<Tag, runir::RoleValueMapContainmentTag>
                 || std::same_as<Tag, runir::RoleValueMapEqualityTag>)
    {
        return make_view(get_data().rhs, *m_context);
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}

#endif
