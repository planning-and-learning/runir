#include "runir/kr/dl/query_construction.hpp"

#include "runir/kr/dl/construction_metadata.hpp"
#include "runir/kr/dl/repository.hpp"

#include <cstddef>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

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
    return std::visit([&](auto concrete) -> const urd::flat_vector<urd::index<QueryColumn>>& { return repository[concrete].columns; },
                      repository[index].variant);
}

void prepare(urd::data<Query<>>& data, const ConstructorRepository& repository)
{
    data.is_static = std::visit([&](auto index) { return infer_is_static(repository.view(index)); }, data.variant);
}

void prepare(urd::data<DatalogProgram>& data, const ConstructorRepository& repository)
{
    const auto& datalog = repository.get_datalog_repository();
    auto arities = urd::Store<urd::sequence<std::optional<size_t>>>(data.inputs.size());
    const auto read = [&](tyr::datalog::database::datalog::RelationView relation)
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
    for (const auto rule : datalog.view(data.program).get_rules())
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
        const auto input = repository.view(data.inputs[slot]);
        if (const auto& arity = arities[slot]; arity && *arity != input.get_schema().size())
            throw std::invalid_argument("Datalog program: an input query does not have its relation's arity.");
        data.is_static = data.is_static && input.is_static();
    }
}

}  // namespace runir::kr::dl::detail
