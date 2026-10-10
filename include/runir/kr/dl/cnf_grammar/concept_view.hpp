#ifndef RUNIR_CNF_GRAMMAR_CONCEPT_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_CONCEPT_VIEW_HPP_

#include "runir/kr/dl/cnf_grammar/concept_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/predicate_view.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename Tag, formalism::SymbolContextFor<runir::kr::dl::cnf_grammar::Concept<Family, Tag>> C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
class View<Index<runir::kr::dl::cnf_grammar::Concept<Family, Tag>>, C> :
    public ygg::IndexViewBase<runir::kr::dl::cnf_grammar::Concept<Family, Tag>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::cnf_grammar::Concept<Family, Tag>, C>::IndexViewBase;

    auto get_identifier() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::RegisterTag> || std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>)
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

    auto get_object() const noexcept
        requires std::same_as<Tag, runir::kr::dl::NominalTag>
    {
        return make_view(this->get_data().object, this->get_context().get_planning_repository());
    }

    auto get_objects() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::RoleFillersTag> || std::same_as<Tag, runir::kr::dl::OneOfTag>)
    {
        return make_view(this->get_data().objects, this->get_context().get_planning_repository());
    }

    auto get_n() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
    {
        return this->get_data().n;
    }

    auto get_role() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::RoleFillersTag>)
    {
        return make_view(this->get_data().role, this->get_context());
    }

    auto get_concept() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
    {
        return make_view(this->get_data().concept_, this->get_context());
    }

    auto get_arg() const noexcept
        requires std::same_as<Tag, runir::kr::dl::NegationTag>
    {
        return make_view(this->get_data().arg, this->get_context());
    }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                 || std::same_as<Tag, runir::kr::dl::ValueRestrictionTag> || std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>
                 || std::same_as<Tag, runir::kr::dl::RoleValueMapTag> || std::same_as<Tag, runir::kr::dl::AgreementTag>)
    {
        return make_view(this->get_data().lhs, this->get_context());
    }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                 || std::same_as<Tag, runir::kr::dl::ValueRestrictionTag> || std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>
                 || std::same_as<Tag, runir::kr::dl::RoleValueMapTag> || std::same_as<Tag, runir::kr::dl::AgreementTag>)
    {
        return make_view(this->get_data().rhs, this->get_context());
    }
};

}

#endif
