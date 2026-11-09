#include "runir/kr/dl/query_construction.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

#include "runir/kr/dl/construction_metadata.hpp"
#include "runir/kr/dl/repository.hpp"

#include <cstddef>
#include <ranges>

namespace runir::kr::dl::detail
{

void require_generic_join_order(std::string_view name,
                                const urd::flat_vector<urd::index<QueryColumn>>& order,
                                const urd::Store<urd::sequence<urd::index<QueryColumn>>>& input_columns,
                                const ConstructorRepository& repository)
{
    const auto column_name = [&](urd::index<QueryColumn> column) { return std::string(urd::resolve(column, repository).get_name()); };
    for (size_t i = 0; i < order.size(); ++i)
    {
        if (std::ranges::find(input_columns, order[i]) == input_columns.end())
            throw std::invalid_argument("q_generic_join: " + std::string(name) + " lists column `" + column_name(order[i]) + "`, which no input has.");
        const auto preceding = std::views::take(order, static_cast<std::ptrdiff_t>(i));
        if (std::ranges::find(preceding, order[i]) != preceding.end())
            throw std::invalid_argument("q_generic_join: " + std::string(name) + " lists column `" + column_name(order[i]) + "` twice.");
    }
    for (const auto column : input_columns)
        if (std::ranges::find(order, column) == order.end())
            throw std::invalid_argument("q_generic_join: " + std::string(name) + " is missing column `" + column_name(column) + "`.");
}

const urd::flat_vector<urd::index<QueryColumn>>& get_query_columns(urd::index<Query<>> index, const ConstructorRepository& repository)
{
    return std::visit([&](auto concrete) -> const urd::flat_vector<urd::index<QueryColumn>>& { return repository[concrete].columns; }, repository[index].variant);
}

void prepare(urd::data<Query<>>& data, const ConstructorRepository& repository)
{
    data.is_static = std::visit([&](auto index) { return infer_is_static(repository.view(index)); }, data.variant);
}

void prepare(urd::data<Numerical<SemanticFamily, DistanceTag>>& data, const ConstructorRepository& repository)
{
    const auto columns = [&](const auto& argument)
    {
        return std::visit(
            [&]<typename T>(urd::index<T> index) -> urd::flat_vector<ygg::ColumnLayout>
            {
                if constexpr (std::same_as<T, Query<>>)
                {
                    const auto schema = repository.view(index).get_schema();
                    auto result = urd::flat_vector<ygg::ColumnLayout>();
                    result.set(schema.begin(), schema.end());
                    return result;
                }
                else
                    return lifted_columns(std::same_as<T, Constructor<ConceptTag>> ? 1 : 2);
            },
            argument);
    };
    const auto sources = columns(data.lhs);
    const auto edges = columns(data.mid);
    const auto targets = columns(data.rhs);
    data.plan = tyr::datalog::database::DistancePlan({ sources.data(), sources.size() }, { edges.data(), edges.size() }, { targets.data(), targets.size() }, distance_column);
}

}  // namespace runir::kr::dl::detail
