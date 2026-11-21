#ifndef RUNIR_KR_DL_NUMERICAL_BUILDER_HPP_
#define RUNIR_KR_DL_NUMERICAL_BUILDER_HPP_

#include "runir/kr/dl/numerical_data.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of Numerical: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Numerical<F, runir::kr::dl::CountTag>, C>
{
    urd::index<runir::kr::dl::Numerical<F, runir::kr::dl::CountTag>> index;
    urd::built_t<typename F::Argument, C> operand;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"operand", &builder::operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Numerical: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Numerical<F, runir::kr::dl::ValueTag>, C>
{
    urd::index<runir::kr::dl::Numerical<F, runir::kr::dl::ValueTag>> index;
    urd::built_t<urd::index<runir::kr::dl::Query<>>, C> operand;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"operand", &builder::operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Numerical: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Numerical<F, runir::kr::dl::DistanceTag>, C>
{
    urd::index<runir::kr::dl::Numerical<F, runir::kr::dl::DistanceTag>> index;
    urd::built_t<typename F::Vertex, C> sources;
    urd::built_t<typename F::Edge, C> edges;
    urd::built_t<typename F::Vertex, C> targets;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"sources", &builder::sources>, urd::field<"edges", &builder::edges>, urd::field<"targets", &builder::targets>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Numerical: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::DistanceTag>, C>
{
    urd::index<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::DistanceTag>> index;
    urd::built_t<runir::kr::dl::SemanticFamily::Vertex, C> sources;
    urd::built_t<runir::kr::dl::SemanticFamily::Edge, C> edges;
    urd::built_t<runir::kr::dl::SemanticFamily::Vertex, C> targets;
    urd::built_t<runir::kr::database::DistancePlan, C> plan {};

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"sources", &builder::sources>,
                           urd::field<"edges", &builder::edges>,
                           urd::field<"targets", &builder::targets>,
                           urd::field<"plan", &builder::plan>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::sources, &builder::edges, &builder::targets> {}; }
};

/// The builder of Numerical: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Numerical<F, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>, C>
{
    urd::index<runir::kr::dl::Numerical<F, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>> index;
    urd::built_t<urd::index<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>>, C> argument;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"argument", &builder::argument>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Numerical: its record's members, each in its built form (urd::built_t).
template<typename F, typename C>
struct builder<runir::kr::dl::Numerical<F, runir::kr::dl::NumericalConstantTag>, C>
{
    urd::index<runir::kr::dl::Numerical<F, runir::kr::dl::NumericalConstantTag>> index;
    urd::built_t<ygg::uint_t, C> value = 0;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"value", &builder::value>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Numerical: its record's members, each in its built form (urd::built_t).
template<typename F, runir::kr::dl::NumericalBinaryTag Tag, typename C>
struct builder<runir::kr::dl::Numerical<F, Tag>, C>
{
    urd::index<runir::kr::dl::Numerical<F, Tag>> index;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::NumericalTag>, C> left_operand;
    urd::built_t<runir::kr::dl::operand_t<F, runir::kr::dl::NumericalTag>, C> right_operand;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"left_operand", &builder::left_operand>, urd::field<"right_operand", &builder::right_operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

}  // namespace urd

#endif
