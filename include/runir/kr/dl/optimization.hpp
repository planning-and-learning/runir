#ifndef RUNIR_KR_DL_OPTIMIZATION_HPP_
#define RUNIR_KR_DL_OPTIMIZATION_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <tyr/declarations.hpp>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/database/optimization/statistics.hpp>
#include <yggdrasil/diagnostics/warnings.hpp>

namespace runir::kr::dl
{

/// Measured sizes of query leaves (atomic state/goal scans and lifted concepts/roles), keyed by leaf query.
template<FamilyTag Family>
struct QueryStatistics
{
    std::optional<size_t> objects;
    ygg::UnorderedMap<FamilyQueryView<Family>, ygg::database::RelationStatistics> leaves;
};

template<FamilyTag Family, typename T>
using FamilyRecordView = ygg::View<ygg::Index<T>, ConstructorRepositoryFor<Family>>;

template<FamilyTag Family>
class QueryOptimizerState;
template<FamilyTag Family>
class QueryStatisticsSamplerState;
template<FamilyTag Family>
class QueryStatisticsCollectorState;

/// Optimizes the queries of DL expressions with yggdrasil's optimizer. It rebuilds records bottom-up, replacing every
/// maximal query (one reached from a non-query record) by its optimized form; records whose children are unchanged keep
/// their index, since reinserting them finds the existing record. Results are memoized, so expressions optimized by one
/// optimizer share rewritten subexpressions. The optimizer runs once per expression, so its state lives in
/// detail/optimization_impl.hpp and is compiled in the library for constructors and queries of each family.
template<FamilyTag Family>
class QueryOptimizer
{
public:
    /// statistics must outlive the optimizer; empty statistics plan every join block structurally.
    QueryOptimizer(ConstructorRepositoryFor<Family>& repository, const QueryStatistics<Family>& statistics);
    QueryOptimizer(QueryOptimizer&&) noexcept;
    QueryOptimizer& operator=(QueryOptimizer&&) noexcept;
    ~QueryOptimizer();

    template<typename T>
    FamilyRecordView<Family, T> optimize(FamilyRecordView<Family, T> view);

private:
    std::unique_ptr<QueryOptimizerState<Family>> m_state;
};

/// Measures fixed query leaves one sample state at a time. Each add evaluates every leaf in one state, so a caller may
/// drop that state's evaluation context and storage right after it: memory stays bounded by one state however many are
/// sampled. The statistics are means over samples and do not depend on their order.
template<FamilyTag Family>
class QueryStatisticsSampler
{
public:
    explicit QueryStatisticsSampler(std::vector<FamilyQueryView<Family>> leaves);
    QueryStatisticsSampler(QueryStatisticsSampler&&) noexcept;
    QueryStatisticsSampler& operator=(QueryStatisticsSampler&&) noexcept;
    ~QueryStatisticsSampler();

    /// Evaluates every leaf in the state of context and adds the sample.
    template<tyr::TaskKind Kind, semantics::StateEvaluationContextConcept<Family, Kind> Context>
    void add(Context& context);

    /// Averages each leaf's row and per-column distinct counts over the added states. The plan is fixed across states,
    /// so the statistics describe a typical state. Leaves empty in every sample keep a size of zero and are reported in
    /// warnings; without samples, the statistics are empty.
    QueryStatistics<Family> finish(ygg::diagnostics::Warnings& warnings) const;

private:
    std::unique_ptr<QueryStatisticsSamplerState<Family>> m_state;
};

/// Measures the leaves of every maximal query reachable from the added expressions, including queries nested inside
/// leaves, in sample states.
template<FamilyTag Family>
class QueryStatisticsCollector
{
public:
    QueryStatisticsCollector();
    QueryStatisticsCollector(QueryStatisticsCollector&&) noexcept;
    QueryStatisticsCollector& operator=(QueryStatisticsCollector&&) noexcept;
    ~QueryStatisticsCollector();

    template<typename T>
    void add(FamilyRecordView<Family, T> expression);

    /// A sampler for the collected leaves, to measure sample states one at a time.
    QueryStatisticsSampler<Family> sampler() const;

    /// The statistics of the collected leaves over contexts, one state evaluation context per sample state, measured
    /// state by state (see QueryStatisticsSampler).
    template<tyr::TaskKind Kind, semantics::StateEvaluationContextConcept<Family, Kind> Context>
    QueryStatistics<Family> measure(const std::vector<Context*>& contexts, ygg::diagnostics::Warnings& warnings) const
    {
        auto sampler = this->sampler();
        for (auto* context : contexts)
            sampler.template add<Kind>(*context);
        return sampler.finish(warnings);
    }

private:
    std::unique_ptr<QueryStatisticsCollectorState<Family>> m_state;
};

/// Optimizes every query inside a DL expression (any constructor or query) with yggdrasil's optimizer and returns the
/// rewritten expression in the same repository. Without statistics, join blocks are planned structurally (Yannakakis or
/// generic join); with statistics for all leaves of a block, they are planned by cost.
template<FamilyTag Family, typename T>
ygg::View<ygg::Index<T>, ConstructorRepositoryFor<Family>> optimize(ygg::View<ygg::Index<T>, ConstructorRepositoryFor<Family>> expression,
                                                                    ConstructorRepositoryFor<Family>& repository,
                                                                    const QueryStatistics<Family>& statistics = {})
{
    if (&expression.get_context() != &repository)
        throw std::invalid_argument("optimize: the expression belongs to another repository.");
    return QueryOptimizer<Family>(repository, statistics).template optimize<T>(expression);
}

/// Measures every query leaf of an expression in sample states and averages the row and per-column distinct counts.
/// contexts holds one state evaluation context per sample state. The plan is fixed across states, so the statistics
/// describe a typical state.
template<tyr::TaskKind Kind, FamilyTag Family, typename T, semantics::StateEvaluationContextConcept<Family, Kind> Context>
QueryStatistics<Family> collect_query_statistics(ygg::View<ygg::Index<T>, ConstructorRepositoryFor<Family>> expression,
                                                 const std::vector<Context*>& contexts,
                                                 ygg::diagnostics::Warnings& warnings)
{
    auto collector = QueryStatisticsCollector<Family>();
    collector.add(expression);
    return collector.template measure<Kind>(contexts, warnings);
}

}  // namespace runir::kr::dl

#ifdef RUNIR_HEADER_INSTANTIATION
#include "runir/kr/dl/detail/optimization_impl.hpp"
#endif

#endif
