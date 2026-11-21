#ifndef RUNIR_KR_DL_BOOLEAN_BUILDER_HPP_
#define RUNIR_KR_DL_BOOLEAN_BUILDER_HPP_

#include "runir/kr/dl/boolean_data.hpp"

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

/// The builder of Boolean: its record's members, each in its built form (urd::built_t).
template<typename F, runir::kr::dl::AtomicTag Tag, typename C>
struct builder<runir::kr::dl::Boolean<F, Tag>, C>
{
    urd::index<runir::kr::dl::Boolean<F, Tag>> index;
    urd::built_t<urd::index<tyr::formalism::Predicate<runir::kr::dl::TagFactKind<Tag>>>, ::tyr::formalism::Repository> predicate;
    urd::built_t<bool, C> polarity = false;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"predicate", &builder::predicate>, urd::field<"polarity", &builder::polarity>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Boolean: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Boolean<F, runir::kr::dl::NonemptyTag>, C>
{
    urd::index<runir::kr::dl::Boolean<F, runir::kr::dl::NonemptyTag>> index;
    urd::built_t<typename F::Argument, C> operand;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"operand", &builder::operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Boolean: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Boolean<F, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>, C>
{
    urd::index<runir::kr::dl::Boolean<F, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>> index;
    urd::built_t<urd::index<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>>, C> argument;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"argument", &builder::argument>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Boolean: its record's members, each in its built form (urd::built_t).
template<typename F, runir::kr::dl::ComparisonTag Tag, typename C>
struct builder<runir::kr::dl::Boolean<F, Tag>, C>
{
    urd::index<runir::kr::dl::Boolean<F, Tag>> index;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::comparison_operand_t<Tag>>, C> left_operand;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::comparison_operand_t<Tag>>, C> right_operand;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"left_operand", &builder::left_operand>, urd::field<"right_operand", &builder::right_operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Boolean: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Boolean<F, runir::kr::dl::BooleanConstantTag>, C>
{
    urd::index<runir::kr::dl::Boolean<F, runir::kr::dl::BooleanConstantTag>> index;
    urd::built_t<bool, C> value = false;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"value", &builder::value>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Boolean: its record's members, each in its built form (urd::built_t).
template<typename F, runir::kr::dl::LogicalBinaryTag Tag, typename C>
struct builder<runir::kr::dl::Boolean<F, Tag>, C>
{
    urd::index<runir::kr::dl::Boolean<F, Tag>> index;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::BooleanTag>, C> left_operand;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::BooleanTag>, C> right_operand;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"left_operand", &builder::left_operand>, urd::field<"right_operand", &builder::right_operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Boolean: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Boolean<F, runir::kr::dl::NotTag>, C>
{
    urd::index<runir::kr::dl::Boolean<F, runir::kr::dl::NotTag>> index;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::BooleanTag>, C> operand;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"operand", &builder::operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

}  // namespace urd

#endif
