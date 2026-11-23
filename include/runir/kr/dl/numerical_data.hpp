#ifndef RUNIR_KR_DL_NUMERICAL_DATA_HPP_
#define RUNIR_KR_DL_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/boolean_data.hpp"

#include <runir/kr/database/semantics/distance_plan.hpp>
#include <urd/elements/flat.hpp>
#include <utility>
#include <yggdrasil/core/config.hpp>

namespace urd
{

template<typename F>
struct data<runir::kr::dl::Numerical<F, runir::kr::dl::CountTag>>
{
    urd::key<runir::kr::dl::Numerical<F, runir::kr::dl::CountTag>> index;
    typename F::Argument operand;

    data() = default;

    explicit data(typename F::Argument operand_) : index(), operand(std::move(operand_)) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"operand", &data::operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::operand> {}; }

    [[nodiscard]] urd::key<runir::kr::dl::Numerical<F, runir::kr::dl::CountTag>> key() const noexcept
    { return urd::key<runir::kr::dl::Numerical<F, runir::kr::dl::CountTag>>(index); }
};

/// n_distance(sources, edges, targets): sources and targets are vertices, edges the middle operand.
template<typename F>
struct data<runir::kr::dl::Numerical<F, runir::kr::dl::DistanceTag>>
{
    urd::key<runir::kr::dl::Numerical<F, runir::kr::dl::DistanceTag>> index;
    typename F::Vertex sources;
    typename F::Edge edges;
    typename F::Vertex targets;

    data() = default;

    data(typename F::Vertex sources_, typename F::Edge edges_, typename F::Vertex targets_) :
        index(),
        sources(std::move(sources_)),
        edges(std::move(edges_)),
        targets(std::move(targets_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"sources", &data::sources>, urd::field<"edges", &data::edges>, urd::field<"targets", &data::targets>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::sources, &data::edges, &data::targets> {}; }

    [[nodiscard]] urd::key<runir::kr::dl::Numerical<F, runir::kr::dl::DistanceTag>> key() const noexcept
    { return urd::key<runir::kr::dl::Numerical<F, runir::kr::dl::DistanceTag>>(index); }
};

/// A semantic distance also holds its plan, derived from its children before it is interned (detail::prepare): sources
/// and targets are k-column vertices, edges 2k columns. The plan is stored but does not identify it, as in the queries.
template<>
struct data<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::DistanceTag>>
{
    urd::key<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::DistanceTag>> index;
    runir::kr::dl::SemanticFamily::Vertex sources;
    runir::kr::dl::SemanticFamily::Edge edges;
    runir::kr::dl::SemanticFamily::Vertex targets;
    runir::kr::database::DistancePlan plan {};

    data() = default;

    data(runir::kr::dl::SemanticFamily::Vertex sources_,
         runir::kr::dl::SemanticFamily::Edge edges_,
         runir::kr::dl::SemanticFamily::Vertex targets_,
         runir::kr::database::DistancePlan plan_) :
        index(),
        sources(std::move(sources_)),
        edges(std::move(edges_)),
        targets(std::move(targets_)),
        plan(std::move(plan_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"sources", &data::sources>,
                           urd::field<"edges", &data::edges>,
                           urd::field<"targets", &data::targets>,
                           urd::field<"plan", &data::plan>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::sources, &data::edges, &data::targets> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::sources, &data::edges, &data::targets, &data::plan> {}; }

    [[nodiscard]] urd::key<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::DistanceTag>> key() const noexcept
    { return urd::key<runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::DistanceTag>>(index); }
};

/// n_value(query): the least value in the query's one column, infinity if it has no row.
template<typename F>
struct data<runir::kr::dl::Numerical<F, runir::kr::dl::ValueTag>>
{
    urd::key<runir::kr::dl::Numerical<F, runir::kr::dl::ValueTag>> index;
    urd::index<runir::kr::dl::Query<>> operand;

    data() = default;

    explicit data(urd::index<runir::kr::dl::Query<>> operand_) : index(), operand(operand_) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"operand", &data::operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::operand> {}; }

    [[nodiscard]] urd::key<runir::kr::dl::Numerical<F, runir::kr::dl::ValueTag>> key() const noexcept
    { return urd::key<runir::kr::dl::Numerical<F, runir::kr::dl::ValueTag>>(index); }
};

template<typename F>
struct data<runir::kr::dl::Numerical<F, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>>
{
    urd::key<runir::kr::dl::Numerical<F, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>> index;
    urd::index<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>> argument;

    data() = default;

    explicit data(urd::index<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>> argument_) : index(), argument(argument_) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"argument", &data::argument>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::argument> {}; }

    [[nodiscard]] urd::key<runir::kr::dl::Numerical<F, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>> key() const noexcept
    { return urd::key<runir::kr::dl::Numerical<F, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>>(index); }
};

template<typename F>
struct data<runir::kr::dl::Numerical<F, runir::kr::dl::NumericalConstantTag>>
{
    urd::key<runir::kr::dl::Numerical<F, runir::kr::dl::NumericalConstantTag>> index;
    ygg::uint_t value = 0;

    data() = default;

    explicit data(ygg::uint_t value_) : index(), value(value_) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"value", &data::value>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::value> {}; }

    [[nodiscard]] urd::key<runir::kr::dl::Numerical<F, runir::kr::dl::NumericalConstantTag>> key() const noexcept
    { return urd::key<runir::kr::dl::Numerical<F, runir::kr::dl::NumericalConstantTag>>(index); }
};

template<typename F, runir::kr::dl::NumericalBinaryTag Tag>
struct data<runir::kr::dl::Numerical<F, Tag>>
{
    urd::key<runir::kr::dl::Numerical<F, Tag>> index;
    runir::kr::dl::operand_t<F, runir::kr::dl::NumericalTag> left_operand;
    runir::kr::dl::operand_t<F, runir::kr::dl::NumericalTag> right_operand;

    data() = default;

    data(runir::kr::dl::operand_t<F, runir::kr::dl::NumericalTag> left_operand_, runir::kr::dl::operand_t<F, runir::kr::dl::NumericalTag> right_operand_) :
        index(),
        left_operand(std::move(left_operand_)),
        right_operand(std::move(right_operand_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"left_operand", &data::left_operand>, urd::field<"right_operand", &data::right_operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::left_operand, &data::right_operand> {}; }

    [[nodiscard]] urd::key<runir::kr::dl::Numerical<F, Tag>> key() const noexcept { return urd::key<runir::kr::dl::Numerical<F, Tag>>(index); }
};

}  // namespace urd

#endif
