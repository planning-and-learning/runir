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

namespace detail
{

inline urd::flat_vector<urd::index<QueryColumn>> query_columns_from_labels(std::span<const urd::index<ygg::Column>> labels)
{
    urd::flat_vector<urd::index<QueryColumn>> columns;
    for (const auto label : labels)
        columns.push_back(urd::index<QueryColumn>(label.value));
    return columns;
}

template<typename Tag>
QueryView<> insert_query(ConstructorRepository& repository, urd::data<Query<Tag>>& data)
{
    const auto concrete = runir::kr::dl::insert(repository, data).first;
    auto wrapper = urd::data<Query<>>();
    wrapper.variant = concrete.index();
    return runir::kr::dl::insert(repository, wrapper).first;
}

/// Inserts the runir counterparts of a plan's nodes; input slots map back to leaves. Returns the roots in plan order.
inline urd::Store<urd::sequence<QueryView<>>>
lower(const tyr::datalog::database::QueryPlan& plan, std::span<const QueryView<>> leaves, ConstructorRepository& repository)
{
    using YggIndex = urd::index<tyr::datalog::database::Query<>>;
    urd::Store<urd::map<YggIndex, QueryView<>>, urd::dense> lowered;
    lowered.reserve(plan.node_count());
    const auto lowered_at = [&](YggIndex index) { return lowered.at(index); };
    const auto child = [&](tyr::datalog::database::QueryView<> node) { return lowered_at(node.index()); };
    for (size_t i = 0; i < plan.node_count(); ++i)
    {
        const auto index = YggIndex(static_cast<ygg::uint_t>(i));
        const auto query = std::visit(
            [&]<typename Tag>(tyr::datalog::database::QueryView<Tag> node) -> QueryView<>
            {
                if constexpr (std::same_as<Tag, tyr::datalog::database::QueryInputTag>)
                    return leaves[node.get_input_slot()];
                else if constexpr (std::same_as<Tag, tyr::datalog::database::QueryJoinTag>)
                {
                    auto data = urd::data<Query<QueryJoinTag>>(child(node.get_lhs()).index(), child(node.get_rhs()).index());
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, tyr::datalog::database::QueryUnionTag>)
                {
                    auto data = urd::data<Query<QueryUnionTag>>(child(node.get_lhs()).index(), child(node.get_rhs()).index());
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, tyr::datalog::database::QueryDifferenceTag>)
                {
                    auto data = urd::data<Query<QueryDifferenceTag>>(child(node.get_lhs()).index(), child(node.get_rhs()).index());
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, tyr::datalog::database::QueryProjectTag>)
                {
                    auto data = urd::data<Query<QueryProjectTag>>(child(node.get_arg()).index(), query_columns_from_labels(node.get_labels()));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, tyr::datalog::database::QueryRenameTag>)
                {
                    auto data = urd::data<Query<QueryRenameTag>>(child(node.get_arg()).index(), query_columns_from_labels(node.get_labels()));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, tyr::datalog::database::QuerySelectEqualTag>)
                {
                    auto data = urd::data<Query<QuerySelectEqualTag>>(child(node.get_arg()).index(),
                                                                      urd::index<QueryColumn>(node.get_lhs_column().value),
                                                                      urd::index<QueryColumn>(node.get_rhs_column().value));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, tyr::datalog::database::QuerySelectValueTag>)
                {
                    auto data = urd::data<Query<QuerySelectValueTag>>(child(node.get_arg()).index(),
                                                                      urd::index<QueryColumn>(node.get_column().value),
                                                                      urd::index<tyr::formalism::Object>(ygg::decode_value<ygg::uint_t>(node.get_constant())));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, tyr::datalog::database::QueryGenericJoinTag>)
                {
                    const auto inputs = node.get_inputs();
                    if (inputs.empty())
                        throw std::logic_error("Query optimization: a generic join without inputs has no runir counterpart.");
                    const auto output_order = query_columns_from_labels(node.get_output_order());
                    if (inputs.size() == 1)
                    {
                        // A single input only reorders its columns.
                        const auto input = lowered_at(inputs.front());
                        if (std::ranges::equal(input.get_columns(), output_order, {}, [](auto column) { return column.index(); }))
                            return input;
                        auto data = urd::data<Query<QueryProjectTag>>(input.index(), output_order);
                        return insert_query(repository, data);
                    }
                    urd::flat_vector<urd::index<Query<>>> args;
                    for (const auto input : inputs)
                        args.push_back(lowered_at(input).index());
                    auto data = urd::data<Query<QueryGenericJoinTag>>(std::move(args), query_columns_from_labels(node.get_variable_order()), output_order);
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, tyr::datalog::database::QueryEmptyTag>)
                {
                    urd::flat_vector<urd::index<QueryColumn>> columns;
                    for (const auto column : node.columns())
                    {
                        if (column.type != ygg::type_id<urd::index<tyr::formalism::Object>>)
                            throw std::logic_error("Query optimization: an empty relation has a non-object column.");
                        columns.push_back(urd::index<QueryColumn>(column.label.value));
                    }
                    auto data = urd::data<Query<QueryEmptyTag>>(std::move(columns));
                    return insert_query(repository, data);
                }
                else
                    throw std::logic_error("Query optimization: runir queries have no distance relations.");
            },
            plan[index].get_variant());
        lowered.insert_or_assign(index, query);
    }
    urd::Store<urd::sequence<QueryView<>>> roots;
    for (const auto root : plan.roots())
        roots.push_back(lowered.at(root));
    return roots;
}

}  // namespace detail

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
    {
        if constexpr (std::same_as<T, Query<>>)
            return optimize_root(view, std::nullopt);
        else
            return memoized(view);
    }

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
    RecordView<Boolean<SemanticFamily, NonemptyTag>> rewrite(RecordView<Boolean<SemanticFamily, NonemptyTag>> view)
    {
        return std::visit(
            [&]<typename V>(V arg) -> RecordView<Boolean<SemanticFamily, NonemptyTag>>
            {
                if constexpr (std::same_as<V, QueryView<>>)
                {
                    auto data = urd::data<Boolean<SemanticFamily, NonemptyTag>>();
                    data.arg = optimize_root(arg, Labels {}).index();
                    return runir::kr::dl::insert(*m_repository, data).first;
                }
                else
                    return rewrite<Boolean<SemanticFamily, NonemptyTag>>(view);
            },
            view.get_arg());
    }

    /// The edges of a distance are evaluated per vertex, with the source or the target columns bound, never fully.
    RecordView<Numerical<SemanticFamily, DistanceTag>> rewrite(RecordView<Numerical<SemanticFamily, DistanceTag>> view)
    {
        using Data = urd::data<Numerical<SemanticFamily, DistanceTag>>;
        auto data = Data(optimize_argument<typename Data::Vertex>(view.get_lhs(), tyr::datalog::database::Evaluation::full),
                         optimize_argument<typename Data::Edge>(view.get_mid(), tyr::datalog::database::Evaluation::bound),
                         optimize_argument<typename Data::Vertex>(view.get_rhs(), tyr::datalog::database::Evaluation::full));
        return runir::kr::dl::insert(*m_repository, data).first;
    }

    /// The edges of a must-pass query are explored per vertex, with the key and the source columns bound, never fully.
    RecordView<Query<QueryMustPassTag>> rewrite(RecordView<Query<QueryMustPassTag>> view)
    {
        auto data = urd::data<Query<QueryMustPassTag>>(view.data().keys,
                                                       optimize_root(view.get_sources(), std::nullopt).index(),
                                                       optimize_root(view.get_edges(), std::nullopt, tyr::datalog::database::Evaluation::bound).index(),
                                                       optimize_root(view.get_targets(), std::nullopt).index(),
                                                       optimize_root(view.get_probes(), std::nullopt).index());
        return runir::kr::dl::insert(*m_repository, data).first;
    }

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
                              tyr::datalog::database::Evaluation evaluation = tyr::datalog::database::Evaluation::full)
    {
        const auto key = std::pair { root, required.value_or(Labels {}) };
        auto& optimized = evaluation == tyr::datalog::database::Evaluation::bound ? m_bound : required ? m_projected : m_optimized;
        if (const auto it = optimized.find(key); it != optimized.end())
            return it->second;
        auto translator = detail::QueryTranslator();
        auto translated = translator.translate(root);
        if (required)
            translated = translator.project(translated, *required);
        auto statistics = tyr::datalog::database::Statistics();
        statistics.objects = m_statistics->objects;
        const auto slot_leaves = translator.get_leaves(root.context());
        auto leaves = urd::Store<urd::sequence<QueryView<>>>();
        for (size_t slot = 0; slot < slot_leaves.size(); ++slot)
        {
            const auto leaf = slot_leaves[slot];
            if (const auto it = m_statistics->leaves.find(leaf); it != m_statistics->leaves.end())
                statistics.inputs.emplace(slot, it->second);
            // Leaves are not optimized themselves, but their concept/role arguments may hold queries.
            leaves.push_back(memoized(leaf));
        }
        const auto plan = tyr::datalog::database::optimize(std::span<const tyr::datalog::database::QueryView<>>(&translated, 1),
                                                           std::span<const tyr::datalog::database::Evaluation>(&evaluation, 1),
                                                           statistics);
        const auto result = detail::lower(plan, leaves, *m_repository).front();
        optimized.emplace(key, result);
        return result;
    }

    ConstructorRepository* m_repository;
    const QueryStatistics* m_statistics;
    urd::Store<urd::map<runir::kr::detail::RecordKey, ygg::uint_t>> m_rewritten;  ///< Rewritten record index by record.
    urd::Store<urd::map<std::pair<QueryView<>, Labels>, QueryView<>>> m_optimized;
    urd::Store<urd::map<std::pair<QueryView<>, Labels>, QueryView<>>> m_projected;
    urd::Store<urd::map<std::pair<QueryView<>, Labels>, QueryView<>>> m_bound;
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
        m_objects = std::max(m_objects.value_or(0), static_cast<size_t>(semantics::detail::num_objects<Kind>(context)));
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
        if (!m_visited.insert(runir::kr::detail::record_key<ConstructorRepositoryTypes>(view)).second)
            return;
        if constexpr (std::same_as<T, Query<>>)
        {
            auto translator = detail::QueryTranslator();
            translator.translate(view);
            for (const auto leaf : translator.get_leaves(view.context()))
                if (m_leaves.insert(leaf).second)
                    // A leaf wrapper's concrete leaf: walk its concept/role argument.
                    for_each_child(leaf, [&]<typename U>(RecordView<U> concrete) { collect_children(concrete); });
        }
        else
            collect_children(view);
    }

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
