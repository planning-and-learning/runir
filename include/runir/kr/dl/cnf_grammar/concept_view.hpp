#ifndef RUNIR_CNF_GRAMMAR_CONCEPT_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_CONCEPT_VIEW_HPP_

#include "runir/kr/dl/cnf_grammar/concept_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/predicate_view.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
class View<Index<runir::kr::dl::cnf_grammar::Concept<Family, Tag>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::dl::cnf_grammar::Concept<Family, Tag>> m_handle;

public:
    View(Index<runir::kr::dl::cnf_grammar::Concept<Family, Tag>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }

    auto get_identifier() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::RegisterTag> || std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>)
    {
        return get_data().identifier;
    }

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

    auto get_object() const noexcept
        requires std::same_as<Tag, runir::kr::dl::NominalTag>
    {
        return make_view(get_data().object, m_context->get_planning_repository());
    }

    auto get_objects() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::RoleFillersTag> || std::same_as<Tag, runir::kr::dl::OneOfTag>)
    {
        return make_view(get_data().objects, m_context->get_planning_repository());
    }

    auto get_n() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
    {
        return get_data().n;
    }

    auto get_role() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::RoleFillersTag>)
    {
        return make_view(get_data().role, *m_context);
    }

    auto get_concept() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
    {
        return make_view(get_data().concept_, *m_context);
    }

    auto get_arg() const noexcept
        requires std::same_as<Tag, runir::kr::dl::NegationTag>
    {
        return make_view(get_data().arg, *m_context);
    }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                 || std::same_as<Tag, runir::kr::dl::ValueRestrictionTag> || std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>
                 || std::same_as<Tag, runir::kr::dl::RoleValueMapTag> || std::same_as<Tag, runir::kr::dl::AgreementTag>)
    {
        return make_view(get_data().lhs, *m_context);
    }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                 || std::same_as<Tag, runir::kr::dl::ValueRestrictionTag> || std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>
                 || std::same_as<Tag, runir::kr::dl::RoleValueMapTag> || std::same_as<Tag, runir::kr::dl::AgreementTag>)
    {
        return make_view(get_data().rhs, *m_context);
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}

#endif
