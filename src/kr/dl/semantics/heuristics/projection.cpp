#include "runir/kr/dl/semantics/heuristics/projection.hpp"

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>

namespace runir::kr::dl::semantics::heuristics
{

std::optional<EdgeAbstraction> abstract_edges(dl::detail::QueryTranslator& translator,
                                              tyr::datalog::database::QueryView<> edges,
                                              std::span<const ygg::ColumnLayout> from,
                                              std::span<const ygg::ColumnLayout> to)
{
    const auto k = from.size();
    auto projections = urd::Store<urd::sequence<tyr::datalog::database::QueryView<>>>();
    for (size_t i = 0; i < k; ++i)
        projections.push_back(translator.project(edges, std::array { from[i].label, to[i].label }));
    const auto& slots = translator.get_slots();
    const auto relaxation = translator.relax(projections, [&](size_t slot) { return slots[slot].is_static; });

    auto roots = urd::Store<urd::sequence<tyr::datalog::database::QueryView<>>>();
    auto patterns = urd::Store<urd::sequence<size_t>>();
    for (size_t i = 0; i < k; ++i)
        if (const auto& relaxed = relaxation.queries[i]; relaxed && relaxed->columns().size() == 2)
        {
            roots.push_back(*relaxed);
            patterns.push_back(i);
        }
    if (roots.empty())
        return std::nullopt;
    return EdgeAbstraction { tyr::datalog::database::compile(roots), std::move(patterns), relaxation.slots };
}

std::optional<EdgeAbstraction>
abstract_edges(dl::detail::QueryTranslator& translator, tyr::datalog::database::QueryView<> edges, const tyr::datalog::database::DistancePlan& plan)
{
    const auto columns = plan.edge_columns();
    return abstract_edges(translator, edges, columns.first(plan.arity()), columns.subspan(plan.arity()));
}

}  // namespace runir::kr::dl::semantics::heuristics
