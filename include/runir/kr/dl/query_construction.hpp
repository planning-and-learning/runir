#ifndef RUNIR_KR_DL_QUERY_CONSTRUCTION_HPP_
#define RUNIR_KR_DL_QUERY_CONSTRUCTION_HPP_

#include "runir/kr/dl/query_view.hpp"

#include <algorithm>
#include <concepts>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tyr/formalism/predicate_view.hpp>
#include <variant>
#include <vector>
#include <yggdrasil/database/semantics/distance.hpp>
#include <yggdrasil/database/semantics/generic_join.hpp>
#include <yggdrasil/database/semantics/must_pass_plan.hpp>
#include <yggdrasil/database/semantics/plans.hpp>
#include <yggdrasil/database/syntax/columns.hpp>

namespace runir::kr::dl::detail
{

inline std::vector<ygg::Index<ygg::database::Column>> query_labels(const ygg::IndexList<QueryColumn>& columns)
{
    std::vector<ygg::Index<ygg::database::Column>> labels;
    labels.reserve(columns.size());
    for (const auto column : columns)
        labels.emplace_back(column.get_value());
    return labels;
}

inline ygg::Builder<ygg::database::Columns> query_columns(const ygg::IndexList<QueryColumn>& columns)
{ return ygg::Builder<ygg::database::Columns>(std::span<const ygg::Index<ygg::database::Column>>(query_labels(columns))); }

inline void require_query_arity(size_t actual, size_t expected)
{
    if (actual != expected)
        throw std::invalid_argument("Query: column count does not match arity.");
}

/// Rejects an order that does not list every input column exactly once, naming the offending column.
void require_generic_join_order(std::string_view name,
                                const ygg::IndexList<QueryColumn>& order,
                                const std::vector<ygg::Index<QueryColumn>>& input_columns,
                                const ConstructorRepository& repository);

const ygg::IndexList<QueryColumn>& get_query_columns(ygg::Index<Query<>> index, const ConstructorRepository& repository);

template<typename T, typename Repository>
void prepare(ygg::Data<T>&, const Repository&)
{
}

/// The repository is a template parameter so that the body is checked once the repository is complete.
template<typename Tag, typename Repository>
    requires(!std::same_as<Tag, void>)
void prepare(ygg::Data<Query<Tag>>& data, const Repository& repository)
{
    if constexpr (std::same_as<Tag, QueryJoinTag>)
    {
        const auto lhs = ygg::make_view(data.lhs, repository).get_schema();
        const auto rhs = ygg::make_view(data.rhs, repository).get_schema();
        data.plan = ygg::database::JoinPlan(lhs.span(), rhs.span());
        data.columns.clear();
        data.columns.reserve(data.plan.output_columns().size());
        for (const auto column : data.plan.output_columns())
            data.columns.push_back(ygg::Index<QueryColumn>(column.label.get_value()));
    }
    else if constexpr (std::same_as<Tag, QueryProjectTag>)
    {
        const auto arg = ygg::make_view(data.arg, repository).get_schema();
        data.plan = ygg::database::ProjectionPlan(arg.span(), query_labels(data.columns));
    }
    else if constexpr (std::same_as<Tag, QueryGenericJoinTag>)
    {
        if (data.args.size() < 2)
            throw std::invalid_argument("q_generic_join: requires at least two input queries.");
        std::vector<std::vector<ygg::database::ColumnLayout>> inputs;
        std::vector<ygg::Index<QueryColumn>> input_columns;
        for (const auto arg : data.args)
        {
            const auto schema = ygg::make_view(arg, repository).get_schema().span();
            inputs.emplace_back(schema.begin(), schema.end());
            for (const auto column : get_query_columns(arg, repository))
                if (std::ranges::find(input_columns, column) == input_columns.end())
                    input_columns.push_back(column);
        }
        require_generic_join_order("variable order", data.variable_order, input_columns, repository);
        require_generic_join_order("output order", data.output_order, input_columns, repository);
        data.plan = ygg::database::GenericJoinPlan(inputs, query_labels(data.variable_order), query_labels(data.output_order));
        data.columns = data.output_order;
    }
    else if constexpr (std::same_as<Tag, QueryMustPassTag>)
    {
        const auto column_name = [&](ygg::Index<QueryColumn> column) { return std::string(ygg::make_view(column, repository).get_name()); };
        const auto require_keys = [&](std::string_view argument, ygg::Index<Query<>> query)
        {
            const auto& columns = get_query_columns(query, repository);
            for (size_t i = 0; i < data.keys.size(); ++i)
                if (i >= columns.size() || columns[i] != data.keys[i])
                    throw std::invalid_argument("q_must_pass: the " + std::string(argument) + " must start with the key columns; column "
                                                + std::to_string(i + 1) + " should be `" + column_name(data.keys[i]) + "`.");
        };
        require_keys("sources", data.sources);
        require_keys("edges", data.edges);
        require_keys("targets", data.targets);
        const auto sources = ygg::make_view(data.sources, repository).get_schema();
        const auto edges = ygg::make_view(data.edges, repository).get_schema();
        const auto targets = ygg::make_view(data.targets, repository).get_schema();
        const auto probes = ygg::make_view(data.probes, repository).get_schema();
        if (probes.size() != 1)
            throw std::invalid_argument("q_must_pass: the probes must have one column.");
        data.plan = ygg::database::MustPassPlan(sources.span(), edges.span(), targets.span(), probes.span(), data.keys.size());
        data.columns = data.keys;
        data.columns.push_back(get_query_columns(data.probes, repository).front());
        data.schema.assign(data.plan.output_columns().span());
    }
    else
    {
        if constexpr (std::same_as<Tag, QueryUnionTag> || std::same_as<Tag, QueryDifferenceTag>)
        {
            const auto lhs = ygg::make_view(data.lhs, repository).get_schema();
            const auto rhs = ygg::make_view(data.rhs, repository).get_schema();
            if (!std::ranges::equal(lhs, rhs))
                throw std::invalid_argument("Query: union and difference require identical ordered columns.");
            data.columns = get_query_columns(data.lhs, repository);
            data.schema.assign(lhs.span());
        }
        else if constexpr (std::same_as<Tag, QuerySelectEqualTag> || std::same_as<Tag, QuerySelectValueTag>)
        {
            data.columns = get_query_columns(data.arg, repository);
            data.schema.assign(ygg::make_view(data.arg, repository).get_schema().span());
        }
        else
            data.schema = query_columns(data.columns);

        const auto& columns = data.schema;
        if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
            require_query_arity(columns.size(), ygg::make_view(data.predicate, repository.get_planning_repository()).get_arity());
        else if constexpr (std::same_as<Tag, QueryConceptTag> || std::same_as<Tag, QueryRoleTag>)
            require_query_arity(columns.size(), std::same_as<Tag, QueryConceptTag> ? 1 : 2);
        else if constexpr (std::same_as<Tag, QueryRenameTag>)
            require_query_arity(columns.size(), ygg::make_view(data.arg, repository).get_schema().size());
        else if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
        {
            data.lhs_position = columns.column_index(ygg::Index<ygg::database::Column>(data.lhs_column.get_value()));
            data.rhs_position = columns.column_index(ygg::Index<ygg::database::Column>(data.rhs_column.get_value()));
        }
        else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
            data.position = columns.column_index(ygg::Index<ygg::database::Column>(data.column.get_value()));
    }
}

template<ConceptOrRoleTag Category, typename Repository>
void prepare(ygg::Data<QueryProjection<Category>>& data, const Repository& repository)
{
    const auto columns = query_columns(data.columns);
    require_query_arity(columns.size(), std::same_as<Category, ConceptTag> ? 1 : 2);
    const auto arg = ygg::make_view(data.arg, repository).get_schema();
    data.plan = ygg::database::ProjectionPlan(arg.span(), query_labels(data.columns));
}

/// The distance plan's output column; interned query column labels never reach the maximum.
inline constexpr auto distance_column = ygg::Index<ygg::database::Column>::max();

/// A concept (k = 1) or role (k = 2) lifted into a relation: object columns labeled 0..k-1.
inline ygg::Builder<ygg::database::Columns> lifted_columns(size_t arity)
{
    ygg::Builder<ygg::database::Columns> columns;
    for (size_t i = 0; i < arity; ++i)
        columns.template push_back<ygg::uint_t>(ygg::Index<ygg::database::Column>(i));
    return columns;
}

/// Constructing the plan rejects arguments whose arities are not k, 2k, k.
void prepare(ygg::Data<Numerical<DistanceTag>>& data, const ConstructorRepository& repository);

}  // namespace runir::kr::dl::detail

#endif
