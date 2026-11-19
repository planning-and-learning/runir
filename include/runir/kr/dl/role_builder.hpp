#ifndef RUNIR_KR_DL_ROLE_BUILDER_HPP_
#define RUNIR_KR_DL_ROLE_BUILDER_HPP_

#include "runir/kr/dl/role_data.hpp"

#include <optional>
#include <tyr/formalism/planning/declarations.hpp>
#include <tyr/formalism/predicate_builder.hpp>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of Role: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Role<F, runir::kr::dl::UniversalTag>, C>
{
    urd::index<runir::kr::dl::Role<F, runir::kr::dl::UniversalTag>> index;

    static constexpr auto fields() noexcept { return urd::Fields<> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Role: its record's members, each in its built form (urd::built_t).
template<typename F, typename Tag, typename C>
    requires runir::kr::dl::is_atomic_state_tag_v<Tag>
struct builder<runir::kr::dl::Role<F, Tag>, C>
{
    urd::index<runir::kr::dl::Role<F, Tag>> index;
    urd::built_t<urd::index<tyr::formalism::Predicate<runir::kr::dl::TagFactKind<Tag>>>, ::tyr::formalism::planning::Repository> predicate;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"predicate", &builder::predicate>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Role: its record's members, each in its built form (urd::built_t).
template<typename F, typename Tag, typename C>
    requires runir::kr::dl::is_atomic_goal_tag_v<Tag>
struct builder<runir::kr::dl::Role<F, Tag>, C>
{
    urd::index<runir::kr::dl::Role<F, Tag>> index;
    urd::built_t<urd::index<tyr::formalism::Predicate<runir::kr::dl::TagFactKind<Tag>>>, ::tyr::formalism::planning::Repository> predicate;
    urd::built_t<bool, C> polarity = false;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"predicate", &builder::predicate>, urd::field<"polarity", &builder::polarity>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Role: its record's members, each in its built form (urd::built_t).
template<typename F, runir::kr::dl::RoleBinaryTag Tag, typename C>
struct builder<runir::kr::dl::Role<F, Tag>, C>
{
    urd::index<runir::kr::dl::Role<F, Tag>> index;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>, C> left_operand;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>, C> right_operand;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"left_operand", &builder::left_operand>, urd::field<"right_operand", &builder::right_operand>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Role: its record's members, each in its built form (urd::built_t).
template<typename F, runir::kr::dl::RoleUnaryTag Tag, typename C>
struct builder<runir::kr::dl::Role<F, Tag>, C>
{
    urd::index<runir::kr::dl::Role<F, Tag>> index;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>, C> operand;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"operand", &builder::operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Role: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Role<F, runir::kr::dl::RestrictionTag>, C>
{
    urd::index<runir::kr::dl::Role<F, runir::kr::dl::RestrictionTag>> index;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag>, C> role;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>, C> qualifier;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"role", &builder::role>, urd::field<"qualifier", &builder::qualifier>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Role: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Role<F, runir::kr::dl::IdentityTag>, C>
{
    urd::index<runir::kr::dl::Role<F, runir::kr::dl::IdentityTag>> index;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag>, C> operand;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"operand", &builder::operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Role: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Role<F, runir::kr::dl::RegisterTag>, C>
{
    urd::index<runir::kr::dl::Role<F, runir::kr::dl::RegisterTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Register<runir::kr::dl::RoleTag>>, C> slot;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"slot", &builder::slot>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Role: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Role<F, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>, C>
{
    urd::index<runir::kr::dl::Role<F, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>> index;
    urd::built_t<urd::index<runir::kr::dl::Argument<runir::kr::dl::RoleTag>>, C> argument;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"argument", &builder::argument>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

}  // namespace urd

#endif
