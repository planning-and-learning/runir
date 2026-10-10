#ifndef RUNIR_KR_DL_QUERY_CONSTRUCTION_HPP_
#define RUNIR_KR_DL_QUERY_CONSTRUCTION_HPP_

#include "runir/kr/dl/query_view.hpp"

#include <algorithm>
#include <concepts>
#include <span>
#include <stdexcept>
#include <tyr/formalism/predicate_view.hpp>
#include <variant>
#include <vector>
#include <yggdrasil/database/semantics/distance.hpp>
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

inline ygg::Builder<ygg::database::Columns<QueryValues>> query_columns(const ygg::IndexList<QueryColumn>& columns)
{
    return ygg::Builder<ygg::database::Columns<QueryValues>>(std::span<const ygg::Index<ygg::database::Column>>(query_labels(columns)));
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
        data.plan = ygg::database::JoinPlan<QueryValues>(lhs.span(), rhs.span());
        data.columns.clear();
        data.columns.reserve(data.plan.output_columns().size());
        for (const auto column : data.plan.output_columns())
            data.columns.push_back(ygg::Index<QueryColumn>(column.label.get_value()));
    }
    else if constexpr (std::same_as<Tag, QueryProjectTag>)
    {
        const auto arg = ygg::make_view(data.arg, repository).get_schema();
        data.plan = ygg::database::ProjectionPlan<QueryValues>(arg.span(), query_labels(data.columns));
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

template<FamilyTag Family, ConceptOrRoleTag Category>
void prepare(ygg::Data<QueryProjection<Family, Category>>& data, const ConstructorRepositoryFor<Family>& repository)
{
    const auto columns = query_columns(data.columns);
    require_query_arity(columns.size(), std::same_as<Category, ConceptTag> ? 1 : 2);
    const auto arg = ygg::make_view(data.arg, repository).get_schema();
    data.plan = ygg::database::ProjectionPlan<QueryValues>(arg.span(), query_labels(data.columns));
}

/// The distance plan's output column; interned query column labels never reach the maximum.
inline constexpr auto distance_column = ygg::Index<ygg::database::Column>::max();

/// A concept (k = 1) or role (k = 2) lifted into a relation: object columns labeled 0..k-1.
inline ygg::Builder<ygg::database::Columns<QueryValues>> lifted_columns(size_t arity)
{
    ygg::Builder<ygg::database::Columns<QueryValues>> columns;
    for (size_t i = 0; i < arity; ++i)
        columns.template push_back<ygg::Index<tyr::formalism::Object>>(ygg::Index<ygg::database::Column>(i));
    return columns;
}

/// Constructing the plan rejects arguments whose arities are not k, 2k, k.
template<FamilyTag Family>
void prepare(ygg::Data<Numerical<Family, DistanceTag>>& data, const ConstructorRepositoryFor<Family>& repository)
{
    const auto columns = [&](const auto& argument)
    {
        return std::visit(
            [&]<typename T>(ygg::Index<T> index) -> ygg::Builder<ygg::database::Columns<QueryValues>>
            {
                if constexpr (std::same_as<T, Query<Family>>)
                    return ygg::Builder<ygg::database::Columns<QueryValues>>(ygg::make_view(index, repository).get_schema().span());
                else
                    return lifted_columns(std::same_as<T, Constructor<Family, ConceptTag>> ? 1 : 2);
            },
            argument);
    };
    const auto sources = columns(data.lhs);
    const auto edges = columns(data.mid);
    const auto targets = columns(data.rhs);
    data.plan = ygg::database::DistancePlan<QueryValues>(sources.span(), edges.span(), targets.span(), distance_column);
}

}  // namespace runir::kr::dl::detail

#endif
