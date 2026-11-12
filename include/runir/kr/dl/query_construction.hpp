#ifndef RUNIR_KR_DL_QUERY_CONSTRUCTION_HPP_
#define RUNIR_KR_DL_QUERY_CONSTRUCTION_HPP_

#include "runir/kr/dl/query_view.hpp"

#include <algorithm>
#include <concepts>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tyr/datalog/lifted/database/datalog/view.hpp>
#include <tyr/datalog/lifted/database/semantics/generic_join.hpp>
#include <tyr/datalog/lifted/database/semantics/plans.hpp>
#include <tyr/formalism/predicate_view.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <variant>
#include <vector>
#include <yggdrasil/formalism/relation_view.hpp>

namespace runir::kr::dl::detail
{

inline urd::Store<urd::sequence<urd::index<ygg::Column>>> query_labels(const urd::flat_vector<urd::index<QueryColumn>>& columns)
{
    urd::Store<urd::sequence<urd::index<ygg::Column>>> labels;
    labels.reserve(columns.size());
    for (const auto column : columns)
        labels.emplace_back(column.value);
    return labels;
}

/// Object columns labeled by the query columns, validated.
inline urd::flat_vector<ygg::ColumnLayout> query_columns(const urd::flat_vector<urd::index<QueryColumn>>& columns)
{
    urd::flat_vector<ygg::ColumnLayout> result;
    for (const auto column : columns)
        result.push_back(ygg::ColumnLayout { urd::index<ygg::Column>(column.value), ygg::type_id<urd::index<tyr::formalism::Object>> });
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
        const auto lhs = urd::resolve(data.lhs, repository).get_schema();
        const auto rhs = urd::resolve(data.rhs, repository).get_schema();
        data.plan = tyr::datalog::database::JoinPlan(lhs, rhs);
        data.columns.clear();
        data.columns.reserve(data.plan.output_columns().size());
        for (const auto column : data.plan.output_columns())
            data.columns.push_back(urd::index<QueryColumn>(column.label.value));
    }
    else if constexpr (std::same_as<Tag, QueryProjectTag>)
    {
        const auto arg = urd::resolve(data.arg, repository).get_schema();
        data.plan = tyr::datalog::database::ProjectionPlan(arg, query_labels(data.columns));
    }
    else if constexpr (std::same_as<Tag, QueryGenericJoinTag>)
    {
        if (data.args.size() < 2)
            throw std::invalid_argument("q_generic_join: requires at least two input queries.");
        urd::Store<urd::sequence<urd::Store<urd::sequence<ygg::ColumnLayout>>>> inputs;
        urd::Store<urd::sequence<urd::index<QueryColumn>>> input_columns;
        for (const auto arg : data.args)
        {
            const auto schema = urd::resolve(arg, repository).get_schema();
            inputs.emplace_back(schema.begin(), schema.end());
            for (const auto column : get_query_columns(arg, repository))
                if (std::ranges::find(input_columns, column) == input_columns.end())
                    input_columns.push_back(column);
        }
        require_generic_join_order("variable order", data.variable_order, input_columns, repository);
        require_generic_join_order("output order", data.output_order, input_columns, repository);
        data.plan = tyr::datalog::database::GenericJoinPlan(inputs, query_labels(data.variable_order), query_labels(data.output_order));
        data.columns = data.output_order;
    }
    else if constexpr (std::same_as<Tag, QueryUnionTag> || std::same_as<Tag, QueryDifferenceTag>)
    {
        const auto lhs = urd::resolve(data.lhs, repository).get_schema();
        const auto rhs = urd::resolve(data.rhs, repository).get_schema();
        if (!std::ranges::equal(lhs, rhs))
            throw std::invalid_argument("Query: union and difference require identical ordered columns.");
        data.columns = get_query_columns(data.lhs, repository);
        data.plan = tyr::datalog::database::SetPlan(lhs, rhs);
    }
    else if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
    {
        data.columns = get_query_columns(data.arg, repository);
        data.plan = tyr::datalog::database::SelectEqualPlan(urd::resolve(data.arg, repository).get_schema(),
                                                            urd::index<ygg::Column>(data.lhs_column.value),
                                                            urd::index<ygg::Column>(data.rhs_column.value));
    }
    else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
    {
        data.columns = get_query_columns(data.arg, repository);
        data.plan = tyr::datalog::database::SelectValuePlan(urd::resolve(data.arg, repository).get_schema(),
                                                            urd::index<ygg::Column>(data.column.value),
                                                            ygg::type_id<urd::index<tyr::formalism::Object>>,
                                                            ygg::encode_value(data.object));
    }
    else
    {
        data.schema = query_columns(data.columns);
        const auto arity = data.schema.size();
        if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
            require_query_arity(arity, urd::resolve(data.predicate, repository.get_planning_repository()).get_arity());
        else if constexpr (std::same_as<Tag, QueryConceptTag> || std::same_as<Tag, QueryRoleTag>)
            require_query_arity(arity, std::same_as<Tag, QueryConceptTag> ? 1 : 2);
        else if constexpr (std::same_as<Tag, QueryBooleanTag> || std::same_as<Tag, QueryNumericalTag>)
            require_query_arity(arity, std::same_as<Tag, QueryBooleanTag> ? 0 : 1);
        else if constexpr (std::same_as<Tag, QueryHeadTag>)
            require_query_arity(arity, repository.get_datalog_repository().view(data.relation).get_arity());
        else if constexpr (std::same_as<Tag, QueryRenameTag>)
            require_query_arity(arity, urd::resolve(data.arg, repository).get_schema().size());
    }
}

/// Each input must have the arity of its slot's relation; the program is static if all its inputs are.
void prepare(urd::data<DatalogProgram>& data, const ConstructorRepository& repository);

template<ConceptOrRoleTag Category, typename Repository>
void prepare(urd::data<QueryProjection<Category>>& data, const Repository& repository)
{
    const auto columns = query_columns(data.columns);
    require_query_arity(columns.size(), std::same_as<Category, ConceptTag> ? 1 : 2);
    const auto arg = urd::resolve(data.arg, repository).get_schema();
    data.plan = tyr::datalog::database::ProjectionPlan(arg, query_labels(data.columns));
}

}  // namespace runir::kr::dl::detail

#endif
