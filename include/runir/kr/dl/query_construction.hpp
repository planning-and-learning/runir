#ifndef RUNIR_KR_DL_QUERY_CONSTRUCTION_HPP_
#define RUNIR_KR_DL_QUERY_CONSTRUCTION_HPP_

#include "runir/kr/dl/query_view.hpp"

#include <algorithm>
#include <concepts>
#include <stdexcept>
#include <tyr/formalism/predicate_view.hpp>
#include <variant>
#include <vector>
#include <yggdrasil/database/columns.hpp>

namespace runir::kr::dl::detail
{

inline ygg::database::Columns query_columns(const ygg::IndexList<QueryColumn>& columns)
{
    std::vector<ygg::database::Column> labels;
    labels.reserve(columns.size());
    for (const auto column : columns)
        labels.push_back(column.get_value());
    return ygg::database::Columns(labels);
}

inline void require_query_arity(size_t actual, size_t expected)
{
    if (actual != expected)
        throw std::invalid_argument("Query: column count does not match arity.");
}

template<FamilyTag Family>
const ygg::IndexList<QueryColumn>& get_query_columns(ygg::Index<Query<Family>> index, const ConstructorRepositoryFor<Family>& repository)
{
    return std::visit([&](auto concrete) -> const ygg::IndexList<QueryColumn>& { return repository[concrete].columns; }, repository[index].variant);
}

template<typename T, FamilyTag Family>
void prepare(ygg::Data<T>&, const ConstructorRepositoryFor<Family>&)
{
}

template<FamilyTag Family, typename Tag>
    requires(!std::same_as<Tag, void>)
void prepare(ygg::Data<Query<Family, Tag>>& data, const ConstructorRepositoryFor<Family>& repository)
{
    if constexpr (std::same_as<Tag, QueryJoinTag>)
    {
        const auto lhs = ygg::make_view(data.lhs, repository).get_schema();
        const auto rhs = ygg::make_view(data.rhs, repository).get_schema();
        data.plan = ygg::database::JoinPlan(lhs, rhs);
        data.columns.clear();
        data.columns.reserve(data.plan.output_columns().size());
        for (const auto column : data.plan.output_columns())
            data.columns.push_back(ygg::Index<QueryColumn>(column));
    }
    else if constexpr (std::same_as<Tag, QueryProjectTag>)
    {
        const auto arg = ygg::make_view(data.arg, repository).get_schema();
        const auto columns = query_columns(data.columns);
        data.plan = ygg::database::ProjectionPlan(arg, columns.view());
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
            data.schema.assign(lhs);
        }
        else if constexpr (std::same_as<Tag, QuerySelectEqualTag> || std::same_as<Tag, QuerySelectValueTag>)
        {
            data.columns = get_query_columns(data.arg, repository);
            data.schema.assign(ygg::make_view(data.arg, repository).get_schema());
        }
        else
            data.schema = query_columns(data.columns);

        const auto columns = data.schema.view();
        if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
            require_query_arity(columns.size(), ygg::make_view(data.predicate, repository.get_planning_repository()).get_arity());
        else if constexpr (std::same_as<Tag, QueryConceptTag> || std::same_as<Tag, QueryRoleTag>)
            require_query_arity(columns.size(), std::same_as<Tag, QueryConceptTag> ? 1 : 2);
        else if constexpr (std::same_as<Tag, QueryRenameTag>)
            require_query_arity(columns.size(), ygg::make_view(data.arg, repository).get_schema().size());
        else if constexpr (std::same_as<Tag, QuerySelectEqualTag>)
        {
            data.lhs_position = columns.column_index(data.lhs_column.get_value());
            data.rhs_position = columns.column_index(data.rhs_column.get_value());
        }
        else if constexpr (std::same_as<Tag, QuerySelectValueTag>)
            data.position = columns.column_index(data.column.get_value());
    }
}

template<FamilyTag Family, CategoryTag Category>
void prepare(ygg::Data<QueryProjection<Family, Category>>& data, const ConstructorRepositoryFor<Family>& repository)
{
    static_assert(std::same_as<Category, ConceptTag> || std::same_as<Category, RoleTag>);
    const auto columns = query_columns(data.columns);
    require_query_arity(columns.size(), std::same_as<Category, ConceptTag> ? 1 : 2);
    const auto arg = ygg::make_view(data.arg, repository).get_schema();
    data.plan = ygg::database::ProjectionPlan(arg, columns.view());
}

}  // namespace runir::kr::dl::detail

#endif
