#ifndef RUNIR_SEMANTICS_CONCEPT_VIEW_HPP_
#define RUNIR_SEMANTICS_CONCEPT_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/concept_data.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/register_view.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/object_view.hpp>
#include <tyr/formalism/predicate_view.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename Tag, typename V>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
struct view_mixin<runir::kr::dl::Concept<Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_predicate() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    { return urd::resolve(self().data().predicate, self().context().get_planning_repository()); }

    auto get_polarity() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    { return self().data().polarity; }

    auto get_object() const noexcept
        requires std::same_as<Tag, runir::kr::dl::NominalTag>
    { return urd::resolve(self().data().object, self().context().get_planning_repository()); }

    auto get_register() const noexcept
        requires std::same_as<Tag, runir::kr::dl::RegisterTag>
    { return self().template get<&urd::record_of<V>::reference>(); }

    auto get_argument() const noexcept
        requires std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>
    { return self().template get<&urd::record_of<V>::reference>(); }

    auto get_objects() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::RoleFillersTag> || std::same_as<Tag, runir::kr::dl::OneOfTag>)
    { return urd::resolve(self().data().objects, self().context().get_planning_repository()); }

    auto get_n() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
    { return self().data().n; }

    auto get_role() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::RoleFillersTag>)
    { return self().template get<&urd::record_of<V>::role>(); }

    auto get_concept() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
                 || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
    { return self().template get<&urd::record_of<V>::concept_>(); }

    auto get_arg() const noexcept
        requires std::same_as<Tag, runir::kr::dl::NegationTag>
    { return self().template get<&urd::record_of<V>::arg>(); }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                 || std::same_as<Tag, runir::kr::dl::ValueRestrictionTag> || std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>
                 || std::same_as<Tag, runir::kr::dl::RoleValueMapTag> || std::same_as<Tag, runir::kr::dl::AgreementTag>)
    { return self().template get<&urd::record_of<V>::lhs>(); }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                 || std::same_as<Tag, runir::kr::dl::ValueRestrictionTag> || std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>
                 || std::same_as<Tag, runir::kr::dl::RoleValueMapTag> || std::same_as<Tag, runir::kr::dl::AgreementTag>)
    { return self().template get<&urd::record_of<V>::rhs>(); }
};
}  // namespace urd

#endif
