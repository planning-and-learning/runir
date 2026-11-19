#ifndef RUNIR_KR_DL_CONCEPT_BUILDER_HPP_
#define RUNIR_KR_DL_CONCEPT_BUILDER_HPP_

#include "runir/kr/dl/concept_data.hpp"

#include <optional>
#include <tyr/formalism/object_builder.hpp>
#include <tyr/formalism/planning/declarations.hpp>
#include <tyr/formalism/predicate_builder.hpp>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of Concept: its record's members, each in its built form (urd::built_t).
template<typename F, runir::kr::dl::NullaryConceptTag Tag, typename C>
struct builder<runir::kr::dl::Concept<F, Tag>, C>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;

    static constexpr auto fields() noexcept { return urd::Fields<> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Concept: its record's members, each in its built form (urd::built_t).
template<typename F, typename Tag, typename C>
    requires runir::kr::dl::is_atomic_state_tag_v<Tag>
struct builder<runir::kr::dl::Concept<F, Tag>, C>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;
    urd::built_t<urd::index<tyr::formalism::Predicate<runir::kr::dl::TagFactKind<Tag>>>, ::tyr::formalism::planning::Repository> predicate;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"predicate", &builder::predicate>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Concept: its record's members, each in its built form (urd::built_t).
template<typename F, typename Tag, typename C>
    requires runir::kr::dl::is_atomic_goal_tag_v<Tag>
struct builder<runir::kr::dl::Concept<F, Tag>, C>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;
    urd::built_t<urd::index<tyr::formalism::Predicate<runir::kr::dl::TagFactKind<Tag>>>, ::tyr::formalism::planning::Repository> predicate;
    urd::built_t<bool, C> polarity = false;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"predicate", &builder::predicate>, urd::field<"polarity", &builder::polarity>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Concept: its record's members, each in its built form (urd::built_t).
template<typename F, runir::kr::dl::ConceptBinaryTag Tag, typename C>
struct builder<runir::kr::dl::Concept<F, Tag>, C>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>, C> left_operand;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>, C> right_operand;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"left_operand", &builder::left_operand>, urd::field<"right_operand", &builder::right_operand>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Concept: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Concept<F, runir::kr::dl::NegationTag>, C>
{
    urd::index<runir::kr::dl::Concept<F, runir::kr::dl::NegationTag>> index;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>, C> operand;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"operand", &builder::operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Concept: its record's members, each in its built form (urd::built_t).
template<typename F, runir::kr::dl::QuantificationTag Tag, typename C>
struct builder<runir::kr::dl::Concept<F, Tag>, C>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>, C> role;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>, C> qualifier;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"role", &builder::role>, urd::field<"qualifier", &builder::qualifier>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Concept: its record's members, each in its built form (urd::built_t).
template<typename F, runir::kr::dl::NumberRestrictionTag Tag, typename C>
struct builder<runir::kr::dl::Concept<F, Tag>, C>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;
    urd::built_t<ygg::uint_t, C> n = 0;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>, C> role;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"n", &builder::n>, urd::field<"role", &builder::role>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Concept: its record's members, each in its built form (urd::built_t).
template<typename F, runir::kr::dl::QualifiedNumberRestrictionTag Tag, typename C>
struct builder<runir::kr::dl::Concept<F, Tag>, C>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;
    urd::built_t<ygg::uint_t, C> n = 0;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>, C> role;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>, C> qualifier;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"n", &builder::n>, urd::field<"role", &builder::role>, urd::field<"qualifier", &builder::qualifier>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Concept: its record's members, each in its built form (urd::built_t).
template<typename F, runir::kr::dl::RoleComparisonTag Tag, typename C>
struct builder<runir::kr::dl::Concept<F, Tag>, C>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>, C> left_operand;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>, C> right_operand;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"left_operand", &builder::left_operand>, urd::field<"right_operand", &builder::right_operand>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Concept: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Concept<F, runir::kr::dl::RoleFillersTag>, C>
{
    urd::index<runir::kr::dl::Concept<F, runir::kr::dl::RoleFillersTag>> index;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>, C> role;
    urd::built_t<urd::flat_vector<urd::index<tyr::formalism::Object>>, ::tyr::formalism::planning::Repository> objects;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"role", &builder::role>, urd::field<"objects", &builder::objects>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Concept: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Concept<F, runir::kr::dl::OneOfTag>, C>
{
    urd::index<runir::kr::dl::Concept<F, runir::kr::dl::OneOfTag>> index;
    urd::built_t<urd::flat_vector<urd::index<tyr::formalism::Object>>, ::tyr::formalism::planning::Repository> objects;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"objects", &builder::objects>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Concept: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Concept<F, runir::kr::dl::NominalTag>, C>
{
    urd::index<runir::kr::dl::Concept<F, runir::kr::dl::NominalTag>> index;
    urd::built_t<urd::index<tyr::formalism::Object>, ::tyr::formalism::planning::Repository> object;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"object", &builder::object>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Concept: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Concept<F, runir::kr::dl::RegisterTag>, C>
{
    urd::index<runir::kr::dl::Concept<F, runir::kr::dl::RegisterTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>, C> slot;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"slot", &builder::slot>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Concept: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Concept<F, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>, C>
{
    urd::index<runir::kr::dl::Concept<F, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>> index;
    urd::built_t<urd::index<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>>, C> argument;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"argument", &builder::argument>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

}  // namespace urd

#endif
