#include "runir/kr/dl/query_construction.hpp"

#include "runir/kr/dl/construction_metadata.hpp"
#include "runir/kr/dl/repository.hpp"

#include <cstddef>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <vector>

namespace runir::kr::dl::detail
{

void require_generic_join_order(std::string_view name,
                                const ygg::IndexList<QueryColumn>& order,
                                const std::vector<ygg::Index<QueryColumn>>& input_columns,
                                const ConstructorRepository& repository)
{
    const auto column_name = [&](ygg::Index<QueryColumn> column) { return std::string(ygg::make_view(column, repository).get_name()); };
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

const ygg::IndexList<QueryColumn>& get_query_columns(ygg::Index<Query<>> index, const ConstructorRepository& repository)
{
    return std::visit([&](auto concrete) -> const ygg::IndexList<QueryColumn>& { return repository[concrete].columns; }, repository[index].variant);
}

void prepare(ygg::Data<DatalogProgram>& data, const ConstructorRepository& repository)
{
    const auto program = ygg::database::datalog::ProgramView(data.program, repository.get_datalog_repository());
    auto arities = std::vector<std::optional<size_t>>(data.inputs.size());
    const auto read = [&](ygg::database::datalog::RelationView relation)
    {
        if (!relation.is_input())
            return;
        const auto slot = relation.get_input_slot();
        if (slot >= data.inputs.size())
            throw std::invalid_argument("Datalog program: an input relation has no input query.");
        arities[slot] = relation.get_arity();
    };
    const auto atoms = [&](auto range)
    {
        for (const auto atom : range)
            read(atom.get_relation());
    };
    for (const auto rule : program.get_rules())
    {
        atoms(rule.get_atoms());
        atoms(rule.get_negated());
        for (const auto aggregate : rule.get_aggregates())
        {
            atoms(aggregate.get_atoms());
            atoms(aggregate.get_negated());
        }
    }
    data.is_static = true;
    for (size_t slot = 0; slot < data.inputs.size(); ++slot)
    {
        const auto input = ygg::make_view(data.inputs[slot], repository);
        if (const auto& arity = arities[slot]; arity && *arity != input.get_schema().size())

            throw std::invalid_argument("Datalog program: an input query does not have its relation's arity.");
        data.is_static = data.is_static && input.is_static();
    }
}

void prepare(ygg::Data<Query<>>& data, const ConstructorRepository& repository)
{
    data.is_static = std::visit([&](auto index) { return infer_is_static(ygg::make_view(index, repository)); }, data.variant);
}

void prepare(ygg::Data<Numerical<DistanceTag>>& data, const ConstructorRepository& repository)
{
    const auto columns = [&](const auto& argument)
    {
        return std::visit(
            [&]<typename T>(ygg::Index<T> index) -> ygg::Builder<ygg::database::Columns>
            {
                if constexpr (std::same_as<T, Query<>>)
                    return ygg::Builder<ygg::database::Columns>(ygg::make_view(index, repository).get_schema().span());
                else
                    return lifted_columns(std::same_as<T, Constructor<ConceptTag>> ? 1 : 2);
            },
            argument);
    };
    const auto sources = columns(data.lhs);
    const auto edges = columns(data.mid);
    const auto targets = columns(data.rhs);
    data.plan = ygg::database::DistancePlan(sources.span(), edges.span(), targets.span(), distance_column);
}

}  // namespace runir::kr::dl::detail
