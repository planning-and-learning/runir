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
#include <concepts>
#include <cstddef>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/core/children.hpp>
#include <yggdrasil/core/concepts.hpp>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/database/optimization/optimization.hpp>
#include <yggdrasil/database/syntax/query.hpp>
#include <yggdrasil/diagnostics/warnings.hpp>

namespace runir::kr::dl
{

/// Measured sizes of query leaves (atomic state/goal scans and lifted concepts/roles), keyed by leaf query.
struct QueryStatistics
{
    std::optional<size_t> objects;
    ygg::UnorderedMap<QueryView<>, ygg::database::RelationStatistics> leaves;
};

namespace detail
{

inline ygg::IndexList<QueryColumn> query_columns_from_labels(std::span<const ygg::Index<ygg::database::Column>> labels)
{
    ygg::IndexList<QueryColumn> columns;
    for (const auto label : labels)
        columns.push_back(ygg::Index<QueryColumn>(label.get_value()));
    return columns;
}

template<typename Tag>
QueryView<> insert_query(ConstructorRepository& repository, ygg::Data<Query<Tag>>& data)
{
    const auto concrete = runir::kr::dl::insert(repository, data).first;
    auto wrapper = ygg::Data<Query<>>();
    wrapper.variant = concrete.get_index();
    return runir::kr::dl::insert(repository, wrapper).first;
}

/// Inserts the runir counterparts of a plan's nodes; input slots map back to leaves. Returns the roots in plan order.
inline std::vector<QueryView<>> lower(const ygg::database::QueryPlan& plan, std::span<const QueryView<>> leaves, ConstructorRepository& repository)
{
    using YggIndex = ygg::Index<ygg::database::Query<>>;
    std::vector<std::optional<QueryView<>>> lowered(plan.node_count());
    const auto lowered_at = [&](YggIndex index) { return lowered.at(index.get_value()).value(); };
    const auto child = [&](ygg::database::QueryView<> node) { return lowered_at(node.get_index()); };
    for (size_t i = 0; i < plan.node_count(); ++i)
    {
        lowered[i] = ygg::visit(
            [&]<typename Tag>(ygg::database::QueryView<Tag> node) -> QueryView<>
            {
                if constexpr (std::same_as<Tag, ygg::database::QueryInputTag>)
                    return leaves[node.get_input_slot()];
                else if constexpr (std::same_as<Tag, ygg::database::QueryJoinTag>)
                {
                    auto data = ygg::Data<Query<QueryJoinTag>>(child(node.get_lhs()), child(node.get_rhs()));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, ygg::database::QueryUnionTag>)
                {
                    auto data = ygg::Data<Query<QueryUnionTag>>(child(node.get_lhs()), child(node.get_rhs()));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, ygg::database::QueryDifferenceTag>)
                {
                    auto data = ygg::Data<Query<QueryDifferenceTag>>(child(node.get_lhs()), child(node.get_rhs()));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, ygg::database::QueryProjectTag>)
                {
                    auto data = ygg::Data<Query<QueryProjectTag>>(child(node.get_arg()).get_index(), query_columns_from_labels(node.get_labels()));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, ygg::database::QueryRenameTag>)
                {
                    auto data = ygg::Data<Query<QueryRenameTag>>(child(node.get_arg()).get_index(), query_columns_from_labels(node.get_labels()));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, ygg::database::QuerySelectEqualTag>)
                {
                    auto data = ygg::Data<Query<QuerySelectEqualTag>>(child(node.get_arg()).get_index(),
                                                                      ygg::Index<QueryColumn>(node.get_lhs_column().get_value()),
                                                                      ygg::Index<QueryColumn>(node.get_rhs_column().get_value()));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, ygg::database::QuerySelectValueTag>)
                {
                    auto data =
                        ygg::Data<Query<QuerySelectValueTag>>(child(node.get_arg()).get_index(),
                                                              ygg::Index<QueryColumn>(node.get_column().get_value()),
                                                              ygg::Index<tyr::formalism::Object>(ygg::ByteCodec<ygg::uint_t>::decode(node.get_constant())));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, ygg::database::QueryGenericJoinTag>)
                {
                    const auto inputs = node.get_inputs();
                    if (inputs.empty())
                        throw std::logic_error("Query optimization: a generic join without inputs has no runir counterpart.");
                    const auto output_order = query_columns_from_labels(node.get_output_order());
                    if (inputs.size() == 1)
                    {
                        // A single input only reorders its columns.
                        const auto input = lowered_at(inputs.front());
                        if (std::ranges::equal(input.get_columns(), output_order, {}, [](auto column) { return column.get_index(); }))
                            return input;
                        auto data = ygg::Data<Query<QueryProjectTag>>(input.get_index(), output_order);
                        return insert_query(repository, data);
                    }
                    ygg::IndexList<Query<>> args;
                    for (const auto input : inputs)
                        args.push_back(lowered_at(input).get_index());
                    auto data = ygg::Data<Query<QueryGenericJoinTag>>(std::move(args), query_columns_from_labels(node.get_variable_order()), output_order);
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, ygg::database::QueryEmptyTag>)
                {
                    ygg::IndexList<QueryColumn> columns;
                    for (const auto column : node.columns())
                    {
                        if (column.type != ygg::type_id<ygg::uint_t>)
                            throw std::logic_error("Query optimization: an empty relation has a non-object column.");
                        columns.push_back(ygg::Index<QueryColumn>(column.label.get_value()));
                    }
                    auto data = ygg::Data<Query<QueryEmptyTag>>(std::move(columns));
                    return insert_query(repository, data);
                }
                else
                    throw std::logic_error("Query optimization: runir queries have no distance relations.");
            },
            plan[YggIndex(static_cast<ygg::uint_t>(i))].get_variant());
    }
    std::vector<QueryView<>> roots;
    for (const auto root : plan.roots())
        roots.push_back(lowered.at(root.get_value()).value());
    return roots;
}

}  // namespace detail

template<typename T>
using RecordView = ygg::View<ygg::Index<T>, ConstructorRepository>;

/// Optimizes the queries of DL expressions with yggdrasil's optimizer. It rebuilds records bottom-up, replacing every
/// maximal query (one reached from a non-query record) by its optimized form; records whose children are unchanged keep
/// their index, since reinserting them finds the existing record. Results are memoized, so expressions optimized by one
/// optimizer share rewritten subexpressions.
class QueryOptimizer
{
public:
    /// statistics must outlive the optimizer; empty statistics plan every join block structurally.
    QueryOptimizer(ConstructorRepository& repository, const QueryStatistics& statistics) : m_repository(repository), m_statistics(statistics) {}

    template<typename T>
    RecordView<T> optimize(RecordView<T> view)
    {
        if constexpr (std::same_as<T, Query<>>)
            return optimize_root(view, std::nullopt);
        else
            return memoized(view);
    }

private:
    using Labels = std::vector<ygg::Index<ygg::database::Column>>;

    template<typename T>
    RecordView<T> memoized(RecordView<T> view)
    {
        const auto key = runir::kr::detail::record_key<ConstructorRepositoryTypes>(view);
        if (const auto it = m_rewritten.find(key); it != m_rewritten.end())
            return ygg::make_view(ygg::Index<T>(it->second), m_repository);
        const auto result = rewrite(view);
        m_rewritten.emplace(key, result.get_index().get_value());
        return result;
    }

    template<typename T>
    RecordView<T> rewrite(RecordView<T> view)
    {
        auto data = ygg::map_children<ConstructorRepositoryTypes>(view, [&]<typename U>(RecordView<U> child) { return optimize<U>(child); });
        return runir::kr::dl::insert(m_repository, data).first;
    }

    /// Only the projected columns of the query are used, so the optimizer may drop the others early. The projection
    /// itself stays with this record: a top projection onto the same columns is unwrapped.
    template<ConceptOrRoleTag Category>
    RecordView<QueryProjection<Category>> rewrite(RecordView<QueryProjection<Category>> view)
    {
        auto root = optimize_root(view.get_arg(), detail::query_labels(view.get_data().columns));
        root = ygg::visit(
            [&]<typename Tag>(QueryView<Tag> concrete) -> QueryView<>
            {
                if constexpr (std::same_as<Tag, QueryProjectTag>)
                    if (std::ranges::equal(concrete.get_columns(), view.get_columns()))
                        return concrete.get_arg();
                return root;
            },
            root.get_variant());
        auto data = ygg::Data<QueryProjection<Category>>(root.get_index(), view.get_data().columns);
        return runir::kr::dl::insert(m_repository, data).first;
    }

    /// Only emptiness matters, so the optimizer may project the query onto no columns.
    RecordView<Boolean<NonemptyTag>> rewrite(RecordView<Boolean<NonemptyTag>> view)
    {
        return ygg::visit(
            [&]<typename V>(V arg) -> RecordView<Boolean<NonemptyTag>>
            {
                if constexpr (std::same_as<V, QueryView<>>)
                {
                    auto data = ygg::Data<Boolean<NonemptyTag>>();
                    data.arg = optimize_root(arg, Labels {}).get_index();
                    return runir::kr::dl::insert(m_repository, data).first;
                }
                else
                    return rewrite<Boolean<NonemptyTag>>(view);
            },
            view.get_arg());
    }

    /// The edges of a distance are evaluated per vertex, with the source or the target columns bound, never fully.
    RecordView<Numerical<DistanceTag>> rewrite(RecordView<Numerical<DistanceTag>> view)
    {
        using Data = ygg::Data<Numerical<DistanceTag>>;
        auto data = Data(optimize_argument<typename Data::Vertex>(view.get_lhs(), ygg::database::Evaluation::full),
                         optimize_argument<typename Data::Edge>(view.get_mid(), ygg::database::Evaluation::bound),
                         optimize_argument<typename Data::Vertex>(view.get_rhs(), ygg::database::Evaluation::full));
        return runir::kr::dl::insert(m_repository, data).first;
    }

    /// The edges of a must-pass query are explored per vertex, with the key and the source columns bound, never fully.
    RecordView<Query<QueryMustPassTag>> rewrite(RecordView<Query<QueryMustPassTag>> view)
    {
        auto data = ygg::Data<Query<QueryMustPassTag>>(view.get_data().keys,
                                                       optimize_root(view.get_sources(), std::nullopt).get_index(),
                                                       optimize_root(view.get_edges(), std::nullopt, ygg::database::Evaluation::bound).get_index(),
                                                       optimize_root(view.get_targets(), std::nullopt).get_index(),
                                                       optimize_root(view.get_probes(), std::nullopt).get_index());
        return runir::kr::dl::insert(m_repository, data).first;
    }

    /// The index of an optimized argument that is a query evaluated as given or another record.
    template<typename Result, typename Argument>
    Result optimize_argument(const Argument& arg, ygg::database::Evaluation evaluation)
    {
        return ygg::visit(
            [&]<typename V>(V child) -> Result
            {
                if constexpr (std::same_as<V, QueryView<>>)
                    return Result(optimize_root(child, std::nullopt, evaluation).get_index());
                else
                    return Result(optimize(child).get_index());
            },
            arg);
    }

    /// Optimizes a maximal query for its evaluation; with required labels, only those columns of its result are used.
    QueryView<> optimize_root(QueryView<> root, const std::optional<Labels>& required, ygg::database::Evaluation evaluation = ygg::database::Evaluation::full)
    {
        const auto key = std::pair { root, required.value_or(Labels {}) };
        auto& optimized = evaluation == ygg::database::Evaluation::bound ? m_bound : required ? m_projected : m_optimized;
        if (const auto it = optimized.find(key); it != optimized.end())
            return it->second;
        auto translator = detail::QueryTranslator();
        auto translated = translator.translate(root);
        if (required)
            translated = translator.project(translated, *required);
        auto statistics = ygg::database::Statistics();
        statistics.objects = m_statistics.objects;
        const auto slot_leaves = translator.get_leaves(root.get_context());
        auto leaves = std::vector<QueryView<>>();
        for (size_t slot = 0; slot < slot_leaves.size(); ++slot)
        {
            const auto leaf = slot_leaves[slot];
            if (const auto it = m_statistics.leaves.find(leaf); it != m_statistics.leaves.end())
                statistics.inputs.emplace(slot, it->second);
            // Leaves are not optimized themselves, but their concept/role arguments may hold queries.
            leaves.push_back(memoized(leaf));
        }
        const auto plan = ygg::database::optimize(std::span<const ygg::database::QueryView<>>(&translated, 1),
                                                               std::span<const ygg::database::Evaluation>(&evaluation, 1),
                                                               statistics);
        const auto result = detail::lower(plan, leaves, m_repository).front();
        optimized.emplace(key, result);
        return result;
    }

    ConstructorRepository& m_repository;
    const QueryStatistics& m_statistics;
    ygg::UnorderedMap<runir::kr::detail::RecordKey, ygg::uint_t> m_rewritten;  ///< Rewritten record index by record.
    ygg::UnorderedMap<std::pair<QueryView<>, Labels>, QueryView<>> m_optimized;
    ygg::UnorderedMap<std::pair<QueryView<>, Labels>, QueryView<>> m_projected;
    ygg::UnorderedMap<std::pair<QueryView<>, Labels>, QueryView<>> m_bound;
};

/// Measures fixed query leaves one sample state at a time. Each add evaluates every leaf in one state, so a caller may
/// drop that state's evaluation context and storage right after it: memory stays bounded by one state however many are
/// sampled. The statistics are means over samples and do not depend on their order.
class QueryStatisticsSampler
{
public:
    explicit QueryStatisticsSampler(std::vector<QueryView<>> leaves) : m_leaves(std::move(leaves)) {}

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
    std::vector<QueryView<>> m_leaves;
    ygg::database::StatisticsAccumulator<QueryView<>> m_accumulator;
    std::optional<size_t> m_objects;
};

/// Measures the leaves of every maximal query reachable from the added expressions, including queries nested inside
/// leaves, in sample states.
class QueryStatisticsCollector
{
public:
    template<typename T>
    void add(RecordView<T> expression)
    {
        collect<T>(expression);
    }

    /// A sampler for the collected leaves, to measure sample states one at a time.
    QueryStatisticsSampler sampler() const { return QueryStatisticsSampler(m_leaves); }

    /// The statistics of the collected leaves over contexts, one state evaluation context per sample state, measured
    /// state by state (see QueryStatisticsSampler).
    template<tyr::TaskKind Kind, semantics::StateEvaluationContextConcept<Kind> Context>
    QueryStatistics measure(const std::vector<Context*>& contexts, ygg::diagnostics::Warnings& warnings) const
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
            for (const auto leaf : translator.get_leaves(view.get_context()))
                if (std::ranges::find(m_leaves, leaf) == m_leaves.end())
                {
                    m_leaves.push_back(leaf);
                    // A leaf wrapper's concrete leaf: walk its concept/role argument.
                    for_each_child(leaf, [&]<typename U>(RecordView<U> concrete) { collect_children(concrete); });
                }
        }
        else
            collect_children(view);
    }

    template<typename T, typename F>
    static void for_each_child(RecordView<T> view, F&& f)
    {
        ygg::for_each_child<ConstructorRepositoryTypes>(view, std::forward<F>(f));
    }

    template<typename T>
    void collect_children(RecordView<T> view)
    {
        for_each_child(view, [&]<typename U>(RecordView<U> child) { collect<U>(child); });
    }

    ygg::UnorderedSet<runir::kr::detail::RecordKey> m_visited;
    std::vector<QueryView<>> m_leaves;
};

/// Optimizes every query inside a DL expression (any constructor or query) with yggdrasil's optimizer and returns the
/// rewritten expression in the same repository. Without statistics, join blocks are planned structurally (Yannakakis or
/// generic join); with statistics for all leaves of a block, they are planned by cost.
template<typename T>
ygg::View<ygg::Index<T>, ConstructorRepository>
optimize(ygg::View<ygg::Index<T>, ConstructorRepository> expression, ConstructorRepository& repository, const QueryStatistics& statistics = {})
{
    if (&expression.get_context() != &repository)
        throw std::invalid_argument("optimize: the expression belongs to another repository.");
    return QueryOptimizer(repository, statistics).template optimize<T>(expression);
}

/// Measures every query leaf of an expression in sample states and averages the row and per-column distinct counts.
/// contexts holds one state evaluation context per sample state. The plan is fixed across states, so the statistics
/// describe a typical state.
template<tyr::TaskKind Kind, typename T, semantics::StateEvaluationContextConcept<Kind> Context>
QueryStatistics collect_query_statistics(ygg::View<ygg::Index<T>, ConstructorRepository> expression,
                                         const std::vector<Context*>& contexts,
                                         ygg::diagnostics::Warnings& warnings)
{
    auto collector = QueryStatisticsCollector();
    collector.add(expression);
    return collector.template measure<Kind>(contexts, warnings);
}

}  // namespace runir::kr::dl

#endif
