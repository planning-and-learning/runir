#ifndef RUNIR_KR_DL_QUERY_DATA_HPP_
#define RUNIR_KR_DL_QUERY_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <boost/mp11.hpp>
#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <cstddef>
#include <optional>
#include <tuple>
#include <tyr/datalog/lifted/database/semantics/generic_join_plan.hpp>
#include <tyr/datalog/lifted/database/semantics/must_pass_plan.hpp>
#include <tyr/datalog/lifted/database/semantics/plans.hpp>
#include <tyr/formalism/declarations.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace runir::kr::dl
{

/// The concrete query a query holds: one of the query constructors.
using QueryVariant = urd::index_variant<boost::mp11::mp_transform<Query, QueryConstructorTags>>;

}  // namespace runir::kr::dl

namespace urd
{

template<>
struct data<runir::kr::dl::QueryColumn>
{
    urd::index<runir::kr::dl::QueryColumn> index;
    urd::flat_string name {};

    data() = default;
    explicit data(urd::flat_string name_) : index(), name(std::move(name_)) {}
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"name", &data::name>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::name> {}; }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::name> {}; }
};

// A query's derived fields (its schema, or its columns and plan, and a query's is_static) are computed from its other
// fields and its children before it is interned (detail::prepare). They are stored, but do not identify it.
template<>
struct data<runir::kr::dl::Query<>>
{
    urd::index<runir::kr::dl::Query<>> index;
    runir::kr::dl::QueryVariant variant;
    bool is_static = false;

    data() = default;
    data(runir::kr::dl::QueryVariant variant_, bool is_static_) : index(), variant(std::move(variant_)), is_static(is_static_) {}
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"variant", &data::variant>, urd::field<"is_static", &data::is_static>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::variant> {}; }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::variant, &data::is_static> {}; }
};

template<typename T>
struct data<runir::kr::dl::Query<runir::kr::dl::AtomicStateTag<T>>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::AtomicStateTag<T>>> index;
    urd::index<tyr::formalism::Predicate<T>> predicate {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    urd::flat_vector<ygg::ColumnLayout> schema {};

    data() = default;
    data(urd::index<tyr::formalism::Predicate<T>> predicate_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_,
         urd::flat_vector<ygg::ColumnLayout> schema_) :
        index(),
        predicate(predicate_),
        columns(std::move(columns_)),
        schema(std::move(schema_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"predicate", &data::predicate>, urd::field<"columns", &data::columns>, urd::field<"schema", &data::schema>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::predicate, &data::columns> {}; }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::predicate, &data::columns, &data::schema> {}; }
};

template<typename T>
struct data<runir::kr::dl::Query<runir::kr::dl::AtomicGoalTag<T>>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::AtomicGoalTag<T>>> index;
    urd::index<tyr::formalism::Predicate<T>> predicate {};
    bool polarity {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    urd::flat_vector<ygg::ColumnLayout> schema {};

    data() = default;
    data(urd::index<tyr::formalism::Predicate<T>> predicate_,
         bool polarity_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_,
         urd::flat_vector<ygg::ColumnLayout> schema_) :
        index(),
        predicate(predicate_),
        polarity(polarity_),
        columns(std::move(columns_)),
        schema(std::move(schema_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"predicate", &data::predicate>,
                           urd::field<"polarity", &data::polarity>,
                           urd::field<"columns", &data::columns>,
                           urd::field<"schema", &data::schema>> {};
    }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::predicate, &data::polarity, &data::columns> {}; }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::predicate, &data::polarity, &data::columns, &data::schema> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryConceptTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryConceptTag>> index;
    urd::index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>> operand {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    urd::flat_vector<ygg::ColumnLayout> schema {};

    data() = default;
    data(urd::index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>> operand_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_,
         urd::flat_vector<ygg::ColumnLayout> schema_) :
        index(),
        operand(operand_),
        columns(std::move(columns_)),
        schema(std::move(schema_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &data::operand>, urd::field<"columns", &data::columns>, urd::field<"schema", &data::schema>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::operand, &data::columns> {}; }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::operand, &data::columns, &data::schema> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryRoleTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryRoleTag>> index;
    urd::index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>> operand {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    urd::flat_vector<ygg::ColumnLayout> schema {};

    data() = default;
    data(urd::index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>> operand_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_,
         urd::flat_vector<ygg::ColumnLayout> schema_) :
        index(),
        operand(operand_),
        columns(std::move(columns_)),
        schema(std::move(schema_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &data::operand>, urd::field<"columns", &data::columns>, urd::field<"schema", &data::schema>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::operand, &data::columns> {}; }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::operand, &data::columns, &data::schema> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryJoinTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryJoinTag>> index;
    urd::index<runir::kr::dl::Query<>> left_operand {};
    urd::index<runir::kr::dl::Query<>> right_operand {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    tyr::datalog::database::JoinPlan plan {};

    data() = default;
    data(urd::index<runir::kr::dl::Query<>> left_operand_,
         urd::index<runir::kr::dl::Query<>> right_operand_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_,
         tyr::datalog::database::JoinPlan plan_) :
        index(),
        left_operand(left_operand_),
        right_operand(right_operand_),
        columns(std::move(columns_)),
        plan(std::move(plan_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"left_operand", &data::left_operand>,
                           urd::field<"right_operand", &data::right_operand>,
                           urd::field<"columns", &data::columns>,
                           urd::field<"plan", &data::plan>> {};
    }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::left_operand, &data::right_operand> {}; }
    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::left_operand, &data::right_operand, &data::columns, &data::plan> {}; }
};

template<typename Tag>
    requires boost::mp11::mp_contains<runir::kr::dl::QuerySetTags, Tag>::value
struct data<runir::kr::dl::Query<Tag>>
{
    urd::index<runir::kr::dl::Query<Tag>> index;
    urd::index<runir::kr::dl::Query<>> left_operand {};
    urd::index<runir::kr::dl::Query<>> right_operand {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    tyr::datalog::database::SetPlan plan {};

    data() = default;
    data(urd::index<runir::kr::dl::Query<>> left_operand_,
         urd::index<runir::kr::dl::Query<>> right_operand_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_,
         tyr::datalog::database::SetPlan plan_) :
        index(),
        left_operand(left_operand_),
        right_operand(right_operand_),
        columns(std::move(columns_)),
        plan(std::move(plan_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"left_operand", &data::left_operand>,
                           urd::field<"right_operand", &data::right_operand>,
                           urd::field<"columns", &data::columns>,
                           urd::field<"plan", &data::plan>> {};
    }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::left_operand, &data::right_operand> {}; }
    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::left_operand, &data::right_operand, &data::columns, &data::plan> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryProjectTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryProjectTag>> index;
    urd::index<runir::kr::dl::Query<>> operand {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    tyr::datalog::database::ProjectionPlan plan {};

    data() = default;
    data(urd::index<runir::kr::dl::Query<>> operand_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_,
         tyr::datalog::database::ProjectionPlan plan_) :
        index(),
        operand(operand_),
        columns(std::move(columns_)),
        plan(std::move(plan_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &data::operand>, urd::field<"columns", &data::columns>, urd::field<"plan", &data::plan>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::operand, &data::columns> {}; }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::operand, &data::columns, &data::plan> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryRenameTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryRenameTag>> index;
    urd::index<runir::kr::dl::Query<>> operand {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    urd::flat_vector<ygg::ColumnLayout> schema {};

    data() = default;
    data(urd::index<runir::kr::dl::Query<>> operand_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_,
         urd::flat_vector<ygg::ColumnLayout> schema_) :
        index(),
        operand(operand_),
        columns(std::move(columns_)),
        schema(std::move(schema_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &data::operand>, urd::field<"columns", &data::columns>, urd::field<"schema", &data::schema>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::operand, &data::columns> {}; }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::operand, &data::columns, &data::schema> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QuerySelectEqualTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QuerySelectEqualTag>> index;
    urd::index<runir::kr::dl::Query<>> operand {};
    urd::index<runir::kr::dl::QueryColumn> left_column {};
    urd::index<runir::kr::dl::QueryColumn> right_column {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    tyr::datalog::database::SelectEqualPlan plan {};

    data() = default;
    data(urd::index<runir::kr::dl::Query<>> operand_,
         urd::index<runir::kr::dl::QueryColumn> left_column_,
         urd::index<runir::kr::dl::QueryColumn> right_column_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_,
         tyr::datalog::database::SelectEqualPlan plan_) :
        index(),
        operand(operand_),
        left_column(left_column_),
        right_column(right_column_),
        columns(std::move(columns_)),
        plan(std::move(plan_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"operand", &data::operand>,
                           urd::field<"left_column", &data::left_column>,
                           urd::field<"right_column", &data::right_column>,
                           urd::field<"columns", &data::columns>,
                           urd::field<"plan", &data::plan>> {};
    }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::operand, &data::left_column, &data::right_column> {}; }
    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::operand, &data::left_column, &data::right_column, &data::columns, &data::plan> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QuerySelectValueTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QuerySelectValueTag>> index;
    urd::index<runir::kr::dl::Query<>> operand {};
    urd::index<runir::kr::dl::QueryColumn> column {};
    urd::index<tyr::formalism::Object> object {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    tyr::datalog::database::SelectValuePlan plan {};

    data() = default;
    data(urd::index<runir::kr::dl::Query<>> operand_,
         urd::index<runir::kr::dl::QueryColumn> column_,
         urd::index<tyr::formalism::Object> object_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_,
         tyr::datalog::database::SelectValuePlan plan_) :
        index(),
        operand(operand_),
        column(column_),
        object(object_),
        columns(std::move(columns_)),
        plan(std::move(plan_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"operand", &data::operand>,
                           urd::field<"column", &data::column>,
                           urd::field<"object", &data::object>,
                           urd::field<"columns", &data::columns>,
                           urd::field<"plan", &data::plan>> {};
    }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::operand, &data::column, &data::object> {}; }
    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::operand, &data::column, &data::object, &data::columns, &data::plan> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryGenericJoinTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryGenericJoinTag>> index;
    urd::flat_vector<urd::index<runir::kr::dl::Query<>>> args {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> variable_order {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> output_order {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    tyr::datalog::database::GenericJoinPlan plan {};

    data() = default;
    data(urd::flat_vector<urd::index<runir::kr::dl::Query<>>> args_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> variable_order_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> output_order_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_,
         tyr::datalog::database::GenericJoinPlan plan_) :
        index(),
        args(std::move(args_)),
        variable_order(std::move(variable_order_)),
        output_order(std::move(output_order_)),
        columns(std::move(columns_)),
        plan(std::move(plan_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"args", &data::args>,
                           urd::field<"variable_order", &data::variable_order>,
                           urd::field<"output_order", &data::output_order>,
                           urd::field<"columns", &data::columns>,
                           urd::field<"plan", &data::plan>> {};
    }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::args, &data::variable_order, &data::output_order> {}; }
    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::args, &data::variable_order, &data::output_order, &data::columns, &data::plan> {}; }
};

/// q_must_pass (keys...) sources edges targets probes: per key, the probe objects that every path from the key's sources
/// to its targets passes in some vertex. Sources, edges and targets start with the key columns; probes have one column.
template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryMustPassTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryMustPassTag>> index;
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> keys {};
    urd::index<runir::kr::dl::Query<>> sources {};
    urd::index<runir::kr::dl::Query<>> edges {};
    urd::index<runir::kr::dl::Query<>> targets {};
    urd::index<runir::kr::dl::Query<>> probes {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    tyr::datalog::database::MustPassPlan plan {};

    data() = default;
    data(urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> keys_,
         urd::index<runir::kr::dl::Query<>> sources_,
         urd::index<runir::kr::dl::Query<>> edges_,
         urd::index<runir::kr::dl::Query<>> targets_,
         urd::index<runir::kr::dl::Query<>> probes_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_,
         tyr::datalog::database::MustPassPlan plan_) :
        index(),
        keys(std::move(keys_)),
        sources(sources_),
        edges(edges_),
        targets(targets_),
        probes(probes_),
        columns(std::move(columns_)),
        plan(std::move(plan_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"keys", &data::keys>,
                           urd::field<"sources", &data::sources>,
                           urd::field<"edges", &data::edges>,
                           urd::field<"targets", &data::targets>,
                           urd::field<"probes", &data::probes>,
                           urd::field<"columns", &data::columns>,
                           urd::field<"plan", &data::plan>> {};
    }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::keys, &data::sources, &data::edges, &data::targets, &data::probes> {}; }
    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::keys, &data::sources, &data::edges, &data::targets, &data::probes, &data::columns, &data::plan> {}; }
};

template<>
struct data<runir::kr::dl::Query<runir::kr::dl::QueryEmptyTag>>
{
    urd::index<runir::kr::dl::Query<runir::kr::dl::QueryEmptyTag>> index;
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    urd::flat_vector<ygg::ColumnLayout> schema {};

    data() = default;
    data(urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_, urd::flat_vector<ygg::ColumnLayout> schema_) :
        index(),
        columns(std::move(columns_)),
        schema(std::move(schema_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"columns", &data::columns>, urd::field<"schema", &data::schema>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::columns> {}; }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::columns, &data::schema> {}; }
};

template<runir::kr::dl::ConceptOrRoleTag Category>
struct data<runir::kr::dl::QueryProjection<Category>>
{
    urd::index<runir::kr::dl::QueryProjection<Category>> index;
    urd::index<runir::kr::dl::Query<>> operand {};
    urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns {};
    tyr::datalog::database::ProjectionPlan plan {};

    data() = default;
    data(urd::index<runir::kr::dl::Query<>> operand_,
         urd::flat_vector<urd::index<runir::kr::dl::QueryColumn>> columns_,
         tyr::datalog::database::ProjectionPlan plan_) :
        index(),
        operand(operand_),
        columns(std::move(columns_)),
        plan(std::move(plan_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &data::operand>, urd::field<"columns", &data::columns>, urd::field<"plan", &data::plan>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::operand, &data::columns> {}; }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::operand, &data::columns, &data::plan> {}; }
};

}  // namespace urd

#endif
