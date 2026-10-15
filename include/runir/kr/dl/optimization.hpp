#ifndef RUNIR_KR_DL_OPTIMIZATION_HPP_
#define RUNIR_KR_DL_OPTIMIZATION_HPP_

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
template<FamilyTag Family>
struct QueryStatistics
{
    std::optional<size_t> objects;
    ygg::UnorderedMap<FamilyQueryView<Family>, ygg::database::RelationStatistics> leaves;
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

template<FamilyTag Family, typename Tag>
FamilyQueryView<Family> insert_query(ConstructorRepositoryFor<Family>& repository, ygg::Data<Query<Family, Tag>>& data)
{
    const auto concrete = runir::kr::dl::insert(repository, data).first;
    auto wrapper = ygg::Data<Query<Family>>();
    wrapper.variant = concrete.get_index();
    return runir::kr::dl::insert(repository, wrapper).first;
}

/// Inserts the runir counterparts of a plan's nodes; input slots map back to leaves. Returns the roots in plan order.
template<FamilyTag Family>
std::vector<FamilyQueryView<Family>>
lower(const ygg::database::QueryPlan<QueryValues>& plan, std::span<const FamilyQueryView<Family>> leaves, ConstructorRepositoryFor<Family>& repository)
{
    using YggIndex = ygg::Index<ygg::database::Query<QueryValues>>;
    std::vector<std::optional<FamilyQueryView<Family>>> lowered(plan.node_count());
    const auto lowered_at = [&](YggIndex index) { return lowered.at(index.get_value()).value(); };
    const auto child = [&](ygg::database::QueryView<QueryValues> node) { return lowered_at(node.get_index()); };
    for (size_t i = 0; i < plan.node_count(); ++i)
    {
        lowered[i] = ygg::visit(
            [&]<typename Tag>(ygg::database::QueryView<QueryValues, Tag> node) -> FamilyQueryView<Family>
            {
                if constexpr (std::same_as<Tag, ygg::database::QueryInputTag>)
                    return leaves[node.get_input_slot()];
                else if constexpr (std::same_as<Tag, ygg::database::QueryJoinTag>)
                {
                    auto data = ygg::Data<Query<Family, QueryJoinTag>>(child(node.get_lhs()), child(node.get_rhs()));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, ygg::database::QueryUnionTag>)
                {
                    auto data = ygg::Data<Query<Family, QueryUnionTag>>(child(node.get_lhs()), child(node.get_rhs()));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, ygg::database::QueryDifferenceTag>)
                {
                    auto data = ygg::Data<Query<Family, QueryDifferenceTag>>(child(node.get_lhs()), child(node.get_rhs()));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, ygg::database::QueryProjectTag>)
                {
                    auto data = ygg::Data<Query<Family, QueryProjectTag>>(child(node.get_arg()).get_index(), query_columns_from_labels(node.get_labels()));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, ygg::database::QueryRenameTag>)
                {
                    auto data = ygg::Data<Query<Family, QueryRenameTag>>(child(node.get_arg()).get_index(), query_columns_from_labels(node.get_labels()));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, ygg::database::QuerySelectEqualTag>)
                {
                    auto data = ygg::Data<Query<Family, QuerySelectEqualTag>>(child(node.get_arg()).get_index(),
                                                                              ygg::Index<QueryColumn>(node.get_lhs_column().get_value()),
                                                                              ygg::Index<QueryColumn>(node.get_rhs_column().get_value()));
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, ygg::database::QuerySelectValueTag>)
                {
                    auto data = ygg::Data<Query<Family, QuerySelectValueTag>>(
                        child(node.get_arg()).get_index(),
                        ygg::Index<QueryColumn>(node.get_column().get_value()),
                        ygg::database::ColumnCodec<ygg::Index<tyr::formalism::Object>>::decode(node.get_constant()));
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
                        auto data = ygg::Data<Query<Family, QueryProjectTag>>(input.get_index(), output_order);
                        return insert_query(repository, data);
                    }
                    ygg::IndexList<Query<Family>> args;
                    for (const auto input : inputs)
                        args.push_back(lowered_at(input).get_index());
                    auto data =
                        ygg::Data<Query<Family, QueryGenericJoinTag>>(std::move(args), query_columns_from_labels(node.get_variable_order()), output_order);
                    return insert_query(repository, data);
                }
                else if constexpr (std::same_as<Tag, ygg::database::QueryEmptyTag>)
                {
                    ygg::IndexList<QueryColumn> columns;
                    for (const auto column : node.columns())
                    {
                        if (column.type != ygg::database::column_type<QueryValues, ygg::Index<tyr::formalism::Object>>)
                            throw std::logic_error("Query optimization: an empty relation has a non-object column.");
                        columns.push_back(ygg::Index<QueryColumn>(column.label.get_value()));
                    }
                    auto data = ygg::Data<Query<Family, QueryEmptyTag>>(std::move(columns));
                    return insert_query(repository, data);
                }
                else
                    throw std::logic_error("Query optimization: runir queries have no distance relations.");
            },
            plan[YggIndex(static_cast<ygg::uint_t>(i))].get_variant());
    }
    std::vector<FamilyQueryView<Family>> roots;
    for (const auto root : plan.roots())
        roots.push_back(lowered.at(root.get_value()).value());
    return roots;
}

}  // namespace detail

template<FamilyTag Family, typename T>
using FamilyRecordView = ygg::View<ygg::Index<T>, ConstructorRepositoryFor<Family>>;

/// Optimizes the queries of DL expressions with yggdrasil's optimizer. It rebuilds records bottom-up, replacing every
/// maximal query (one reached from a non-query record) by its optimized form; records whose children are unchanged keep
/// their index, since reinserting them finds the existing record. Results are memoized, so expressions optimized by one
/// optimizer share rewritten subexpressions.
template<FamilyTag Family>
class QueryOptimizer
{
    template<typename T>
    using RewriteMap = ygg::UnorderedMap<FamilyRecordView<Family, T>, FamilyRecordView<Family, T>>;

public:
    /// statistics must outlive the optimizer; empty statistics plan every join block structurally.
    QueryOptimizer(ConstructorRepositoryFor<Family>& repository, const QueryStatistics<Family>& statistics) : m_repository(repository), m_statistics(statistics)
    {
    }

    template<typename T>
    FamilyRecordView<Family, T> optimize(FamilyRecordView<Family, T> view)
    {
        if constexpr (std::same_as<T, Query<Family>>)
            return optimize_root(view, std::nullopt);
        else
            return memoized(view);
    }

private:
    using Labels = std::vector<ygg::Index<ygg::database::Column>>;

    template<typename T>
    FamilyRecordView<Family, T> memoized(FamilyRecordView<Family, T> view)
    {
        auto& rewritten = std::get<RewriteMap<T>>(m_rewritten);
        if (const auto it = rewritten.find(view); it != rewritten.end())
            return it->second;
        const auto result = rewrite(view);
        rewritten.emplace(view, result);
        return result;
    }

    template<typename T>
    FamilyRecordView<Family, T> rewrite(FamilyRecordView<Family, T> view)
    {
        auto data =
            ygg::map_children<FamilyConstructorRepositoryTypes<Family>>(view,
                                                                        [&]<typename U>(FamilyRecordView<Family, U> child) { return optimize<U>(child); });
        return runir::kr::dl::insert(m_repository, data).first;
    }

    /// Only the projected columns of the query are used, so the optimizer may drop the others early. The projection
    /// itself stays with this record: a top projection onto the same columns is unwrapped.
    template<ConceptOrRoleTag Category>
    FamilyRecordView<Family, QueryProjection<Family, Category>> rewrite(FamilyRecordView<Family, QueryProjection<Family, Category>> view)
    {
        auto root = optimize_root(view.get_arg(), detail::query_labels(view.get_data().columns));
        root = ygg::visit(
            [&]<typename Tag>(FamilyQueryView<Family, Tag> concrete) -> FamilyQueryView<Family>
            {
                if constexpr (std::same_as<Tag, QueryProjectTag>)
                    if (std::ranges::equal(concrete.get_columns(), view.get_columns()))
                        return concrete.get_arg();
                return root;
            },
            root.get_variant());
        auto data = ygg::Data<QueryProjection<Family, Category>>(root.get_index(), view.get_data().columns);
        return runir::kr::dl::insert(m_repository, data).first;
    }

    /// Only emptiness matters, so the optimizer may project the query onto no columns.
    FamilyRecordView<Family, Boolean<Family, NonemptyTag>> rewrite(FamilyRecordView<Family, Boolean<Family, NonemptyTag>> view)
    {
        return ygg::visit(
            [&]<typename V>(V arg) -> FamilyRecordView<Family, Boolean<Family, NonemptyTag>>
            {
                if constexpr (std::same_as<V, FamilyQueryView<Family>>)
                {
                    auto data = ygg::Data<Boolean<Family, NonemptyTag>>();
                    data.arg = optimize_root(arg, Labels {}).get_index();
                    return runir::kr::dl::insert(m_repository, data).first;
                }
                else
                    return rewrite<Boolean<Family, NonemptyTag>>(view);
            },
            view.get_arg());
    }

    /// The edges of a distance are evaluated per vertex, with the source or the target columns bound, never fully.
    FamilyRecordView<Family, Numerical<Family, DistanceTag>> rewrite(FamilyRecordView<Family, Numerical<Family, DistanceTag>> view)
    {
        using Data = ygg::Data<Numerical<Family, DistanceTag>>;
        auto data = Data(optimize_argument<typename Data::Vertex>(view.get_lhs(), ygg::database::Evaluation::full),
                         optimize_argument<typename Data::Edge>(view.get_mid(), ygg::database::Evaluation::bound),
                         optimize_argument<typename Data::Vertex>(view.get_rhs(), ygg::database::Evaluation::full));
        return runir::kr::dl::insert(m_repository, data).first;
    }

    /// The edges of a must-pass query are explored per vertex, with the key and the source columns bound, never fully.
    FamilyRecordView<Family, Query<Family, QueryMustPassTag>> rewrite(FamilyRecordView<Family, Query<Family, QueryMustPassTag>> view)
    {
        auto data = ygg::Data<Query<Family, QueryMustPassTag>>(view.get_data().keys,
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
                if constexpr (std::same_as<V, FamilyQueryView<Family>>)
                    return Result(optimize_root(child, std::nullopt, evaluation).get_index());
                else
                    return Result(optimize(child).get_index());
            },
            arg);
    }

    /// Optimizes a maximal query for its evaluation; with required labels, only those columns of its result are used.
    FamilyQueryView<Family>
    optimize_root(FamilyQueryView<Family> root, const std::optional<Labels>& required, ygg::database::Evaluation evaluation = ygg::database::Evaluation::full)
    {
        const auto key = std::pair { root, required.value_or(Labels {}) };
        auto& optimized = evaluation == ygg::database::Evaluation::bound ? m_bound : required ? m_projected : m_optimized;
        if (const auto it = optimized.find(key); it != optimized.end())
            return it->second;
        auto translator = detail::QueryTranslator<Family>();
        auto translated = translator.translate(root);
        if (required)
            translated = translator.project(translated, *required);
        auto statistics = ygg::database::Statistics<QueryValues>();
        statistics.objects = m_statistics.objects;
        const auto slot_leaves = translator.get_leaves(root.get_context());
        auto leaves = std::vector<FamilyQueryView<Family>>();
        for (size_t slot = 0; slot < slot_leaves.size(); ++slot)
        {
            const auto leaf = slot_leaves[slot];
            if (const auto it = m_statistics.leaves.find(leaf); it != m_statistics.leaves.end())
                statistics.inputs.emplace(slot, it->second);
            // Leaves are not optimized themselves, but their concept/role arguments may hold queries.
            leaves.push_back(memoized(leaf));
        }
        const auto plan = ygg::database::optimize<QueryValues>(std::span<const ygg::database::QueryView<QueryValues>>(&translated, 1),
                                                               std::span<const ygg::database::Evaluation>(&evaluation, 1),
                                                               statistics);
        const auto result = detail::lower<Family>(plan, leaves, m_repository).front();
        optimized.emplace(key, result);
        return result;
    }

    ConstructorRepositoryFor<Family>& m_repository;
    const QueryStatistics<Family>& m_statistics;
    ygg::ApplyTypeListT<std::tuple, ygg::MapTypeListT<RewriteMap, FamilyConstructorRepositoryTypes<Family>>> m_rewritten;
    ygg::UnorderedMap<std::pair<FamilyQueryView<Family>, Labels>, FamilyQueryView<Family>> m_optimized;
    ygg::UnorderedMap<std::pair<FamilyQueryView<Family>, Labels>, FamilyQueryView<Family>> m_projected;
    ygg::UnorderedMap<std::pair<FamilyQueryView<Family>, Labels>, FamilyQueryView<Family>> m_bound;
};

/// Measures fixed query leaves one sample state at a time. Each add evaluates every leaf in one state, so a caller may
/// drop that state's evaluation context and storage right after it: memory stays bounded by one state however many are
/// sampled. The statistics are means over samples and do not depend on their order.
template<FamilyTag Family>
class QueryStatisticsSampler
{
public:
    explicit QueryStatisticsSampler(std::vector<FamilyQueryView<Family>> leaves) : m_leaves(std::move(leaves)) {}

    /// Evaluates every leaf in the state of context and adds the sample.
    template<tyr::TaskKind Kind, semantics::StateEvaluationContextConcept<Family, Kind> Context>
    void add(Context& context)
    {
        m_objects = std::max(m_objects.value_or(0), static_cast<size_t>(semantics::detail::num_objects<Family, Kind>(context)));
        for (const auto leaf : m_leaves)
            m_accumulator.add(leaf, semantics::evaluate<Kind>(leaf, context));
    }

    /// Averages each leaf's row and per-column distinct counts over the added states. The plan is fixed across states,
    /// so the statistics describe a typical state. Leaves empty in every sample keep a size of zero and are reported in
    /// warnings; without samples, the statistics are empty.
    QueryStatistics<Family> finish(ygg::diagnostics::Warnings& warnings) const
    {
        auto result = QueryStatistics<Family>();
        if (!m_objects)
            return result;
        result.objects = m_objects;
        result.leaves = m_accumulator.finish(warnings, [](FamilyQueryView<Family> leaf) { return fmt::format("{}", leaf); });
        return result;
    }

private:
    std::vector<FamilyQueryView<Family>> m_leaves;
    ygg::database::StatisticsAccumulator<QueryValues, FamilyQueryView<Family>> m_accumulator;
    std::optional<size_t> m_objects;
};

/// Measures the leaves of every maximal query reachable from the added expressions, including queries nested inside
/// leaves, in sample states.
template<FamilyTag Family>
class QueryStatisticsCollector
{
    template<typename T>
    using VisitedSet = ygg::UnorderedSet<FamilyRecordView<Family, T>>;
    template<typename T>
    using RecordView = FamilyRecordView<Family, T>;

public:
    template<typename T>
    void add(FamilyRecordView<Family, T> expression)
    {
        collect<T>(expression);
    }

    /// A sampler for the collected leaves, to measure sample states one at a time.
    QueryStatisticsSampler<Family> sampler() const { return QueryStatisticsSampler<Family>(m_leaves); }

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
    template<typename T>
    void collect(FamilyRecordView<Family, T> view)
    {
        if (!std::get<VisitedSet<T>>(m_visited).insert(view).second)
            return;
        if constexpr (std::same_as<T, Query<Family>>)
        {
            auto translator = detail::QueryTranslator<Family>();
            translator.translate(view);
            for (const auto leaf : translator.get_leaves(view.get_context()))
                if (std::ranges::find(m_leaves, leaf) == m_leaves.end())
                {
                    m_leaves.push_back(leaf);
                    // A leaf wrapper's concrete leaf: walk its concept/role argument.
                    for_each_child(leaf, [&]<typename U>(FamilyRecordView<Family, U> concrete) { collect_children(concrete); });
                }
        }
        else
            collect_children(view);
    }

    template<typename T, ygg::InvocableWithEach<ygg::MapTypeListT<RecordView, FamilyConstructorRepositoryTypes<Family>>> F>
    static void for_each_child(FamilyRecordView<Family, T> view, F&& f)
    {
        ygg::for_each_child<FamilyConstructorRepositoryTypes<Family>>(view, std::forward<F>(f));
    }

    template<typename T>
    void collect_children(FamilyRecordView<Family, T> view)
    {
        for_each_child(view, [&]<typename U>(FamilyRecordView<Family, U> child) { collect<U>(child); });
    }

    ygg::ApplyTypeListT<std::tuple, ygg::MapTypeListT<VisitedSet, FamilyConstructorRepositoryTypes<Family>>> m_visited;
    std::vector<FamilyQueryView<Family>> m_leaves;
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

#endif
