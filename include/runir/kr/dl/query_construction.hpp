#ifndef RUNIR_KR_DL_QUERY_CONSTRUCTION_HPP_
#define RUNIR_KR_DL_QUERY_CONSTRUCTION_HPP_

#include "runir/kr/dl/query_view.hpp"

#include <algorithm>
#include <concepts>
#include <ranges>
#include <runir/kr/database/datalog/view.hpp>
#include <runir/kr/database/semantics/distance.hpp>
#include <runir/kr/database/semantics/generic_join.hpp>
#include <runir/kr/database/semantics/plans.hpp>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tyr/formalism/planning/declarations.hpp>
#include <tyr/formalism/predicate_view.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <variant>
#include <vector>
#include <yggdrasil/formalism/relation_view.hpp>

namespace runir::kr::dl::detail
{

/// The column labels of query columns, given as their indices or their views.
template<std::ranges::sized_range R>
urd::Store<urd::sequence<urd::index<ygg::Column>>> query_labels(const R& columns)
{
    urd::Store<urd::sequence<urd::index<ygg::Column>>> labels;
    labels.reserve(std::ranges::size(columns));
    for (const auto& column : columns)
        labels.emplace_back(urd::stored<urd::index<QueryColumn>>(column).value);
    return labels;
}

/// Object columns labeled by the query columns, validated.
inline urd::flat_vector<urd::data<ygg::ColumnLayout>> query_columns(const urd::flat_vector<urd::index<QueryColumn>>& columns)
{
    urd::flat_vector<urd::data<ygg::ColumnLayout>> result;
    for (const auto column : columns)
        result.push_back(urd::data<ygg::ColumnLayout> { urd::index<ygg::Column>(column.value), ygg::type_id<urd::index<tyr::formalism::Object>> });
    ygg::validate_columns({ result.data(), result.size() });
    return result;
}

inline void require_query_arity(size_t actual, size_t expected)
{
    if (actual != expected)
        throw std::invalid_argument("Query: column count does not match arity.");
}

/// Rejects an order that does not list every input column exactly once, naming the offending column.
void require_generic_join_order(std::string_view name,
                                const urd::flat_vector<urd::index<QueryColumn>>& order,
                                const urd::Store<urd::sequence<urd::index<QueryColumn>>>& input_columns,
                                const ConstructorRepository& repository);

const urd::flat_vector<urd::index<QueryColumn>>& get_query_columns(urd::index<Query<>> index, const ConstructorRepository& repository);

template<typename T, typename Repository>
void prepare(urd::data<T>&, const Repository&)
{
}

/// The repository is a template parameter so that the body is checked once the repository is complete.
template<typename Tag, typename Repository>
    requires(!std::same_as<Tag, void>)
void prepare(urd::data<Query<Tag>>& data, const Repository& repository)
{
    if constexpr (std::same_as<Tag, QueryJoinTag>)
    {
        const auto lhs = schema_of(urd::resolve(data.left_operand, repository));
        const auto rhs = schema_of(urd::resolve(data.right_operand, repository));
        data.plan = runir::kr::database::JoinPlan(lhs, rhs);
        data.columns.clear();
        data.columns.reserve(data.plan.output_columns().size());
        for (const auto column : data.plan.output_columns())
            data.columns.push_back(urd::index<QueryColumn>(column.label.value));
    }
    else if constexpr (std::same_as<Tag, QueryProjectTag>)
    {
        const auto arg = schema_of(urd::resolve(data.operand, repository));
        data.plan = runir::kr::database::ProjectionPlan(arg, query_labels(data.columns));
    }
    else if constexpr (std::same_as<Tag, QueryGenericJoinTag>)
    {
        if (data.args.size() < 2)
            throw std::invalid_argument("q_generic_join: requires at least two input queries.");
        urd::Store<urd::sequence<urd::Store<urd::sequence<urd::data<ygg::ColumnLayout>>>>> inputs;
        urd::Store<urd::sequence<urd::index<QueryColumn>>> input_columns;
        for (const auto arg : data.args)
        {
            const auto schema = schema_of(urd::resolve(arg, repository));
            inputs.emplace_back(schema.begin(), schema.end());
            for (const auto column : get_query_columns(arg, repository))
                if (std::ranges::find(input_columns, column) == input_columns.end())
                    input_columns.push_back(column);
        }
        require_generic_join_order("variable order", data.variable_order, input_columns, repository);
        require_generic_join_order("output order", data.output_order, input_columns, repository);
        data.plan = runir::kr::database::GenericJoinPlan(inputs, query_labels(data.variable_order), query_labels(data.output_order));
        data.columns = data.output_order;
    }
    else if constexpr (std::same_as<Tag, QueryUnionTag> || std::same_as<Tag, QueryDifferenceTag>)
    {
        const auto lhs = schema_of(urd::resolve(data.left_operand, repository));
        const auto rhs = schema_of(urd::resolve(data.right_operand, repository));
        if (!std::ranges::equal(lhs, rhs))
            throw std::invalid_argument("Query: union and difference require identical ordered columns.");
        data.columns = get_query_columns(data.left_operand, repository);
        data.plan = runir::kr::database::SetPlan(lhs, rhs);
    }
    else if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
    {
        data.columns = get_query_columns(data.operand, repository);
        data.plan = runir::kr::database::SelectEqualPlan(schema_of(urd::resolve(data.operand, repository)),
                                                         urd::index<ygg::Column>(data.left_column.value),
                                                         urd::index<ygg::Column>(data.right_column.value));
    }
    else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
    {
        data.columns = get_query_columns(data.operand, repository);
        data.plan = runir::kr::database::SelectValuePlan(schema_of(urd::resolve(data.operand, repository)),
                                                         urd::index<ygg::Column>(data.column.value),
                                                         ygg::type_id<urd::index<tyr::formalism::Object>>,
                                                         ygg::encode_value(data.object));
    }
    else
    {
        data.schema = query_columns(data.columns);
        const auto arity = data.schema.size();
        if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
            require_query_arity(arity, urd::resolve(data.predicate, repository.template get<tyr::formalism::Repository>()).get_arity());
        else if constexpr (std::same_as<Tag, QueryConceptTag> || std::same_as<Tag, QueryRoleTag>)
            require_query_arity(arity, std::same_as<Tag, QueryConceptTag> ? 1 : 2);
        else if constexpr (std::same_as<Tag, QueryBooleanTag> || std::same_as<Tag, QueryNumericalTag>)
            require_query_arity(arity, std::same_as<Tag, QueryBooleanTag> ? 0 : 1);
        else if constexpr (std::same_as<Tag, QueryHeadTag>)
            require_query_arity(arity, repository.get_datalog_repository().view(urd::index<runir::kr::database::datalog::Relation>(data.relation)).get_arity());
        else if constexpr (std::same_as<Tag, QueryRenameTag>)
            require_query_arity(arity, schema_of(urd::resolve(data.operand, repository)).size());
    }
}

/// Each input must have the arity of its slot's relation; the program is static if all its inputs are.
void prepare(urd::data<DatalogProgram>& data, const ConstructorRepository& repository);

template<ConceptOrRoleTag Category, typename Repository>
void prepare(urd::data<QueryProjection<Category>>& data, const Repository& repository)
{
    const auto columns = query_columns(data.columns);
    require_query_arity(columns.size(), std::same_as<Category, ConceptTag> ? 1 : 2);
    const auto arg = schema_of(urd::resolve(data.operand, repository));
    data.plan = runir::kr::database::ProjectionPlan(arg, query_labels(data.columns));
}

/// The distance plan's output column; interned query column labels never reach the maximum.
inline constexpr auto distance_column = urd::index<ygg::Column>();

/// A concept (k = 1) or role (k = 2) lifted into a relation: object columns labeled 0..k-1.
inline urd::flat_vector<urd::data<ygg::ColumnLayout>> lifted_columns(size_t arity)
{
    urd::flat_vector<urd::data<ygg::ColumnLayout>> columns;
    for (size_t i = 0; i < arity; ++i)
        columns.push_back(
            urd::data<ygg::ColumnLayout> { urd::index<ygg::Column>(static_cast<std::uint32_t>(i)), ygg::type_id<urd::index<tyr::formalism::Object>> });
    return columns;
}

/// Constructing the plan rejects arguments whose arities are not k, 2k, k.
void prepare(urd::data<Numerical<SemanticFamily, DistanceTag>>& data, const ConstructorRepository& repository);

}  // namespace runir::kr::dl::detail

#endif
