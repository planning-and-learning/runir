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
                                              runir::kr::database::QueryView<> edges,
                                              std::span<const urd::data<ygg::ColumnLayout>> from,
                                              std::span<const urd::data<ygg::ColumnLayout>> to)
{
    const auto k = from.size();
    auto projections = urd::Store<urd::sequence<runir::kr::database::QueryView<>>>();
    for (size_t i = 0; i < k; ++i)
        projections.push_back(translator.project(edges, std::array { from[i].label, to[i].label }));
    const auto& slots = translator.get_slots();
    const auto relaxation = translator.relax(projections, [&](size_t slot) { return slots[slot].is_static; });

    auto roots = urd::Store<urd::sequence<runir::kr::database::QueryView<>>>();
    auto patterns = urd::Store<urd::sequence<size_t>>();
    for (size_t i = 0; i < k; ++i)
        if (const auto& relaxed = relaxation.queries[i]; relaxed && runir::kr::database::columns_of(*relaxed).size() == 2)
        {
            roots.push_back(*relaxed);
            patterns.push_back(i);
        }
    if (roots.empty())
        return std::nullopt;
    return EdgeAbstraction { runir::kr::database::compile_plan(roots), std::move(patterns), relaxation.slots };
}

std::optional<EdgeAbstraction>
abstract_edges(dl::detail::QueryTranslator& translator, runir::kr::database::QueryView<> edges, const urd::data<runir::kr::database::DistancePlan>& plan)
{
    const auto columns = std::span<const urd::data<ygg::ColumnLayout>>(plan.edge_columns);
    return abstract_edges(translator, edges, columns.first(arity(plan)), columns.subspan(arity(plan)));
}

}  // namespace runir::kr::dl::semantics::heuristics
