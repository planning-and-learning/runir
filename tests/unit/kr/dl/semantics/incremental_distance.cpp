#include <algorithm>
#include <gtest/gtest.h>
#include <limits>
#include <random>
#include <runir/kr/dl/semantics/incremental/detail/distance.hpp>
#include <string>
#include <tyr/formalism/object_data.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <vector>

namespace runir::tests
{
namespace
{

TEST(RunirIncrementalDistance, RepairsMixedChangesAndUndoAgainstFreshBreadthFirstSearch)
{
    namespace dl = kr::dl;
    namespace sem = dl::semantics;
    constexpr ygg::uint_t size = 10;
    constexpr auto infinity = std::numeric_limits<ygg::uint_t>::max();
    auto repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    for (ygg::uint_t i = 0; i < size; ++i)
    {
        auto data = ygg::Data<tyr::formalism::Object>("o" + std::to_string(i));
        (void) repository->insert(data);
    }
    const auto object = [&](ygg::uint_t index) { return ygg::make_view(ygg::Index<tyr::formalism::Object>(index), *repository); };
    auto sources = ygg::Builder<sem::Denotation<dl::ConceptTag>>(size);
    auto edges = ygg::Builder<sem::Denotation<dl::RoleTag>>(size);
    auto targets = ygg::Builder<sem::Denotation<dl::ConceptTag>>(size);
    sources.get().set(0);
    targets.get().set(3);
    edges.get(0).set(1);
    edges.get(0).set(2);
    edges.get(1).set(3);
    edges.get(2).set(3);
    edges.get(3).set(4);
    edges.get(4).set(3);
    edges.get(0).set(5);
    edges.get(5).set(6);
    edges.get(6).set(7);
    edges.get(7).set(4);
    edges.get(8).set(9);
    edges.get(9).set(8);
    auto previous_sources = sources;
    auto previous_edges = edges;
    auto previous_targets = targets;
    auto evaluator = sem::incremental::detail::DistanceEvaluator();
    const auto sources_view = ygg::make_view(sources, *repository);
    const auto edges_view = ygg::make_view(edges, *repository);
    const auto targets_view = ygg::make_view(targets, *repository);
    EXPECT_THROW((void) evaluator.get_result(), std::logic_error);
    evaluator.initialize(sources_view, edges_view, targets_view);
    EXPECT_EQ(evaluator.get_result(), 2);

    const auto reference = [&]
    {
        auto distances = std::vector<ygg::uint_t>(size, infinity);
        auto queue = std::vector<ygg::uint_t>();
        for (ygg::uint_t source = 0; source < size; ++source)
            if (sources.get()[source])
            {
                distances[source] = 0;
                queue.push_back(source);
            }
        for (size_t position = 0; position < queue.size(); ++position)
        {
            const auto source = queue[position];
            for (ygg::uint_t target = 0; target < size; ++target)
                if (edges.get(source)[target] && distances[target] == infinity)
                {
                    distances[target] = distances[source] + 1;
                    queue.push_back(target);
                }
        }
        auto result = infinity;
        for (ygg::uint_t target = 0; target < size; ++target)
            if (targets.get()[target])
                result = std::min(result, distances[target]);
        return result;
    };
    const auto check = [&]
    {
        auto source_delta = sem::incremental::DenotationDelta<dl::ConceptTag>();
        auto edge_delta = sem::incremental::DenotationDelta<dl::RoleTag>();
        auto target_delta = sem::incremental::DenotationDelta<dl::ConceptTag>();
        for (ygg::uint_t from = 0; from < size; ++from)
        {
            if (sources.get()[from] != previous_sources.get()[from])
                (sources.get()[from] ? source_delta.added : source_delta.removed).push_back(object(from));
            if (targets.get()[from] != previous_targets.get()[from])
                (targets.get()[from] ? target_delta.added : target_delta.removed).push_back(object(from));
            for (ygg::uint_t to = 0; to < size; ++to)
                if (edges.get(from)[to] != previous_edges.get(from)[to])
                    (edges.get(from)[to] ? edge_delta.added : edge_delta.removed).emplace_back(object(from), object(to));
        }
        evaluator.update(sources_view, edges_view, targets_view, source_delta, edge_delta, target_delta);
        EXPECT_EQ(evaluator.get_result(), reference());
        previous_sources = sources;
        previous_edges = edges;
        previous_targets = targets;
    };

    edges.get(1).reset(3);  // The other shortest predecessor still supports 3.
    check();
    EXPECT_EQ(evaluator.get_result(), 2);
    edges.get(2).reset(3);  // Repair through the longer boundary path and cycle.
    check();
    EXPECT_EQ(evaluator.get_result(), 5);
    edges.get(0).reset(5);  // An unrooted cycle must become unreachable.
    check();
    EXPECT_EQ(evaluator.get_result(), infinity);
    sources.get().set(4);
    check();
    EXPECT_EQ(evaluator.get_result(), 1);
    sources.get().reset(0);
    targets.get().set(4);
    check();
    EXPECT_EQ(evaluator.get_result(), 0);
    targets.get().reset(4);
    check();
    EXPECT_EQ(evaluator.get_result(), 1);
    sources.get().reset(4);
    sources.get().set(9);
    edges.get(9).set(3);
    check();
    EXPECT_EQ(evaluator.get_result(), 1);
    check();  // Empty deltas leave the retained state intact.

    // Mixed batches include source/target swaps, equal-length alternatives,
    // self loops, cycles, changes in disconnected components, and cancellation.
    auto random = std::mt19937(793);
    for (size_t iteration = 0; iteration < 2000; ++iteration)
    {
        SCOPED_TRACE(iteration);
        const auto saved_sources = sources;
        const auto saved_edges = edges;
        const auto saved_targets = targets;
        const auto changes = 1 + random() % 20;
        for (size_t change = 0; change < changes; ++change)
        {
            const auto from = static_cast<ygg::uint_t>(random() % size);
            const auto to = static_cast<ygg::uint_t>(random() % size);
            switch (random() % 5)
            {
                case 0:
                    if (random() % 2 == 0)
                        sources.get().reset();
                    sources.get()[from] ? sources.get().reset(from) : sources.get().set(from);
                    break;
                case 1:
                    targets.get()[to] ? targets.get().reset(to) : targets.get().set(to);
                    break;
                default:
                    edges.get(from).set(to, random() % 5 == 0);
            }
        }
        check();
        if (iteration % 3 == 0)
        {
            sources = saved_sources;
            edges = saved_edges;
            targets = saved_targets;
            check();
        }
        if (iteration % 17 == 0)
        {
            evaluator.initialize(sources_view, edges_view, targets_view);
            EXPECT_EQ(evaluator.get_result(), reference());
        }
        // Probe every retained distance: a multi-target minimum alone can hide
        // an incorrect distance whenever another target is already a source.
        const auto selected_targets = targets;
        for (ygg::uint_t target = 0; target < size; ++target)
        {
            targets.get().reset();
            targets.get().set(target);
            check();
        }
        targets = selected_targets;
        check();
    }
}

}  // namespace
}  // namespace runir::tests
