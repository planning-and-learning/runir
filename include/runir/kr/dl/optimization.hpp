#ifndef RUNIR_KR_DL_OPTIMIZATION_HPP_
#define RUNIR_KR_DL_OPTIMIZATION_HPP_

#include "runir/kr/detail/record_key.hpp"
#include "runir/kr/dl/query_construction.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/formatter.hpp"
#include "runir/kr/dl/semantics/query_evaluation.hpp"
#include "runir/kr/dl/translations/ygg_query.hpp"

#include <algorithm>
#include <array>
#include <boost/mp11.hpp>
#include <concepts>
#include <cstddef>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <tyr/datalog/lifted/database/optimization/optimization.hpp>
#include <tyr/datalog/lifted/database/syntax/query.hpp>
#include <urd/config.hpp>
#include <urd/elements/children.hpp>
#include <urd/repr/maps/index_map.hpp>
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/indexed.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/diagnostics/warnings.hpp>

namespace runir::kr::dl
{

/// Measured sizes of query leaves (atomic state/goal scans and lifted concepts/roles), keyed by leaf query.
struct QueryStatistics
{
    std::optional<size_t> objects;
    urd::Store<urd::map<QueryView<>, tyr::datalog::database::RelationStatistics>> leaves;
};

template<typename T>
using RecordView = urd::index_view<T, ConstructorRepository>;

/// Optimizes the queries of DL expressions with yggdrasil's optimizer. It rebuilds records bottom-up, replacing every
/// maximal query (one reached from a non-query record) by its optimized form; records whose children are unchanged keep
/// their index, since reinserting them finds the existing record. Results are memoized, so expressions optimized by one
/// optimizer share rewritten subexpressions.
class QueryOptimizer
{
public:
    /// statistics must outlive the optimizer; empty statistics plan every join block structurally.
    QueryOptimizer(ConstructorRepository& repository, const QueryStatistics& statistics) : m_repository(&repository), m_statistics(&statistics) {}

    template<typename T>
    RecordView<T> optimize(RecordView<T> view)
    { return memoized(view); }

private:
    using Labels = urd::Store<urd::sequence<urd::index<ygg::Column>>>;

    template<typename T>
    RecordView<T> memoized(RecordView<T> view)
    {
        const auto key = runir::kr::detail::record_key<ConstructorRepositoryTypes>(view);
        if (const auto it = m_rewritten.find(key); it != m_rewritten.end())
            return m_repository->view(urd::index<T>(it->second));
        const auto result = rewrite(view);
        m_rewritten.emplace(key, result.index().value);
        return result;
    }

    template<typename T>
    RecordView<T> rewrite(RecordView<T> view)
    {
        auto data = urd::map_children<ConstructorRepositoryTypes, T>(view.data(),
                                                                     view.context(),
                                                                     [&]<typename U>(RecordView<U> child) { return optimize<U>(child).index(); });
        return runir::kr::dl::insert(*m_repository, data).first;
    }

    /// Only the projected columns of the query are used, so the optimizer may drop the others early. The projection
    /// itself stays with this record: a top projection onto the same columns is unwrapped.
    template<ConceptOrRoleTag Category>
    RecordView<QueryProjection<Category>> rewrite(RecordView<QueryProjection<Category>> view)
    {
        auto root = optimize_root(view.get_arg(), detail::query_labels(view.data().columns));
        root = std::visit(
            [&]<typename Tag>(QueryView<Tag> concrete) -> QueryView<>
            {
                if constexpr (std::same_as<Tag, QueryProjectTag>)
                    if (std::ranges::equal(concrete.get_columns(), view.get_columns()))
                        return concrete.get_arg();
                return root;
            },
            root.get_variant());
        auto data = urd::data<QueryProjection<Category>>(root.index(), view.data().columns);
        return runir::kr::dl::insert(*m_repository, data).first;
    }

    /// Only emptiness matters, so the optimizer may project the query onto no columns.
    RecordView<Boolean<SemanticFamily, NonemptyTag>> rewrite(RecordView<Boolean<SemanticFamily, NonemptyTag>> view);
    /// The edges of a distance are evaluated per vertex, with the source or the target columns bound, never fully.
    RecordView<Numerical<SemanticFamily, DistanceTag>> rewrite(RecordView<Numerical<SemanticFamily, DistanceTag>> view);
    /// The edges of a must-pass query are explored per vertex, with the key and the source columns bound, never fully.
    RecordView<Query<QueryMustPassTag>> rewrite(RecordView<Query<QueryMustPassTag>> view);

    /// The index of an optimized argument that is a query evaluated as given or another record.
    template<typename Result, typename Argument>
    Result optimize_argument(const Argument& arg, tyr::datalog::database::Evaluation evaluation)
    {
        return std::visit(
            [&]<typename V>(V child) -> Result
            {
                if constexpr (std::same_as<V, QueryView<>>)
                    return Result(optimize_root(child, std::nullopt, evaluation).index());
                else
                    return Result(optimize(child).index());
            },
            arg);
    }

    /// Optimizes a maximal query for its evaluation; with required labels, only those columns of its result are used.
    QueryView<> optimize_root(QueryView<> root,
                              const std::optional<Labels>& required,
                              tyr::datalog::database::Evaluation evaluation = tyr::datalog::database::Evaluation::full);

    ConstructorRepository* m_repository;
    const QueryStatistics* m_statistics;
    urd::Store<urd::map<runir::kr::detail::RecordKey, ygg::uint_t>> m_rewritten;  ///< Rewritten record index by record.
    /// Optimized maximal queries, by query, required labels and evaluation.
    urd::Store<urd::map<std::tuple<QueryView<>, std::optional<Labels>, tyr::datalog::database::Evaluation>, QueryView<>>> m_optimized;
};

/// Measures fixed query leaves one sample state at a time. Each add evaluates every leaf in one state, so a caller may
/// drop that state's evaluation context and storage right after it: memory stays bounded by one state however many are
/// sampled. The statistics are means over samples and do not depend on their order.
class QueryStatisticsSampler
{
public:
    explicit QueryStatisticsSampler(urd::Store<urd::sequence<QueryView<>>> leaves) : m_leaves(std::move(leaves)) {}

    /// Evaluates every leaf in the state of context and adds the sample.
    template<tyr::TaskKind Kind, semantics::StateEvaluationContextConcept<Kind> Context>
    void add(Context& context)
    {
        const auto objects = static_cast<size_t>(semantics::detail::num_objects<Kind>(context));
        m_objects = m_objects ? std::max(*m_objects, objects) : objects;  // none until the first sample
        for (const auto leaf : m_leaves)
            m_accumulator.add(leaf, semantics::evaluate<Kind>(leaf, context));
    }

    /// Averages each leaf's row and per-column distinct counts over the added states. The plan is fixed across states,
    /// so the statistics describe a typical state. Leaves empty in every sample keep a size of zero and are reported in
    /// warnings; without samples, the statistics are empty.
    QueryStatistics finish(ygg::diagnostics::Warnings& warnings) const
    {
        auto result = QueryStatistics();
        if (!m_objects)
            return result;
        result.objects = m_objects;
        result.leaves = m_accumulator.finish(warnings, [](QueryView<> leaf) { return fmt::format("{}", leaf); });
        return result;
    }

private:
    urd::Store<urd::sequence<QueryView<>>> m_leaves;
    tyr::datalog::database::StatisticsAccumulator<QueryView<>> m_accumulator;
    std::optional<size_t> m_objects;
};

/// Measures the leaves of every maximal query reachable from the added expressions, including queries nested inside
/// leaves, in sample states.
class QueryStatisticsCollector
{
public:
    template<typename T>
    void add(RecordView<T> expression)
    { collect<T>(expression); }

    /// A sampler for the collected leaves, to measure sample states one at a time.
    [[nodiscard]] QueryStatisticsSampler sampler() const
    { return QueryStatisticsSampler(urd::Store<urd::sequence<QueryView<>>>(m_leaves.begin(), m_leaves.end())); }

    /// The statistics of the collected leaves over contexts, one state evaluation context per sample state, measured
    /// state by state (see QueryStatisticsSampler).
    template<tyr::TaskKind Kind, semantics::StateEvaluationContextConcept<Kind> Context>
    QueryStatistics measure(const urd::Store<urd::sequence<Context*>>& contexts, ygg::diagnostics::Warnings& warnings) const
    {
        auto sampler = this->sampler();
        for (auto* context : contexts)
            sampler.template add<Kind>(*context);
        return sampler.finish(warnings);
    }

private:
    template<typename T>
    void collect(RecordView<T> view)
    {
        if (first_visit(view))
            collect_children(view);
    }

    template<typename T>
    bool first_visit(RecordView<T> view)
    { return m_visited.insert(runir::kr::detail::record_key<ConstructorRepositoryTypes>(view)).second; }

    template<typename T, typename F>
    static void for_each_child(RecordView<T> view, F&& f)
    { urd::for_each_child<ConstructorRepositoryTypes, T>(view.data(), view.context(), std::forward<F>(f)); }

    template<typename T>
    void collect_children(RecordView<T> view)
    {
        for_each_child(view, [&]<typename U>(RecordView<U> child) { collect<U>(child); });
    }

    urd::Store<urd::set<runir::kr::detail::RecordKey>> m_visited;
    urd::Store<urd::set<QueryView<>>, urd::insertion_ordered> m_leaves { urd::layer { 0 } };
};

// Optimizing or collecting a constructor or a query of any kind visits its operands, of every kind in turn, so the
// library compiles these once instead of every file that optimizes one (brokkr's template rules, rule 24). A query of any
// kind is a maximal query: optimized whole, its leaves collected.
template<>
RecordView<Constructor<ConceptTag>> QueryOptimizer::optimize(RecordView<Constructor<ConceptTag>> view);
template<>
RecordView<Constructor<RoleTag>> QueryOptimizer::optimize(RecordView<Constructor<RoleTag>> view);
template<>
RecordView<Constructor<BooleanTag>> QueryOptimizer::optimize(RecordView<Constructor<BooleanTag>> view);
template<>
RecordView<Constructor<NumericalTag>> QueryOptimizer::optimize(RecordView<Constructor<NumericalTag>> view);
template<>
RecordView<Query<>> QueryOptimizer::optimize(RecordView<Query<>> view);
template<>
void QueryStatisticsCollector::collect(RecordView<Constructor<ConceptTag>> view);
template<>
void QueryStatisticsCollector::collect(RecordView<Constructor<RoleTag>> view);
template<>
void QueryStatisticsCollector::collect(RecordView<Constructor<BooleanTag>> view);
template<>
void QueryStatisticsCollector::collect(RecordView<Constructor<NumericalTag>> view);
template<>
void QueryStatisticsCollector::collect(RecordView<Query<>> view);

/// Optimizes every query inside a DL expression (any constructor or query) with yggdrasil's optimizer and returns the
/// rewritten expression in the same repository. Without statistics, join blocks are planned structurally (Yannakakis or
/// generic join); with statistics for all leaves of a block, they are planned by cost.
template<typename T>
urd::index_view<T, ConstructorRepository>
optimize(urd::index_view<T, ConstructorRepository> expression, ConstructorRepository& repository, const QueryStatistics& statistics = {})
{
    if (&expression.context() != &repository)
        throw std::invalid_argument("optimize: the expression belongs to another repository.");
    return QueryOptimizer(repository, statistics).template optimize<T>(expression);
}

/// Measures every query leaf of an expression in sample states and averages the row and per-column distinct counts.
/// contexts holds one state evaluation context per sample state. The plan is fixed across states, so the statistics
/// describe a typical state.
template<tyr::TaskKind Kind, typename T, semantics::StateEvaluationContextConcept<Kind> Context>
QueryStatistics collect_query_statistics(urd::index_view<T, ConstructorRepository> expression,
                                         const urd::Store<urd::sequence<Context*>>& contexts,
                                         ygg::diagnostics::Warnings& warnings)
{
    auto collector = QueryStatisticsCollector();
    collector.add(expression);
    return collector.template measure<Kind>(contexts, warnings);
}

}  // namespace runir::kr::dl

#endif
