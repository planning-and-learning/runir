#include "runir/kr/dl/optimization.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <variant>

namespace runir::kr::dl
{

namespace
{

urd::flat_vector<urd::index<QueryColumn>> query_columns_from_labels(std::span<const urd::index<ygg::Column>> labels)
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
urd::Store<urd::sequence<QueryView<>>>
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

}  // namespace

template<>
RecordView<Constructor<ConceptTag>> QueryOptimizer::optimize(RecordView<Constructor<ConceptTag>> view)
{ return memoized(view); }
template<>
RecordView<Constructor<RoleTag>> QueryOptimizer::optimize(RecordView<Constructor<RoleTag>> view)
{ return memoized(view); }
template<>
RecordView<Constructor<BooleanTag>> QueryOptimizer::optimize(RecordView<Constructor<BooleanTag>> view)
{ return memoized(view); }
template<>
RecordView<Constructor<NumericalTag>> QueryOptimizer::optimize(RecordView<Constructor<NumericalTag>> view)
{ return memoized(view); }
template<>
RecordView<Query<>> QueryOptimizer::optimize(RecordView<Query<>> view)
{ return optimize_root(view, std::nullopt); }

template<>
void QueryStatisticsCollector::collect(RecordView<Constructor<ConceptTag>> view)
{
    if (first_visit(view))
        collect_children(view);
}
template<>
void QueryStatisticsCollector::collect(RecordView<Constructor<RoleTag>> view)
{
    if (first_visit(view))
        collect_children(view);
}
template<>
void QueryStatisticsCollector::collect(RecordView<Constructor<BooleanTag>> view)
{
    if (first_visit(view))
        collect_children(view);
}
template<>
void QueryStatisticsCollector::collect(RecordView<Constructor<NumericalTag>> view)
{
    if (first_visit(view))
        collect_children(view);
}
template<>
void QueryStatisticsCollector::collect(RecordView<Query<>> view)
{
    if (!first_visit(view))
        return;
    auto translator = detail::QueryTranslator();
    translator.translate(view);
    for (const auto leaf : translator.get_leaves(view.context()))
        if (m_leaves.insert(leaf).second)
            // A leaf wrapper's concrete leaf: walk its concept/role argument.
            for_each_child(leaf, [&]<typename U>(RecordView<U> concrete) { collect_children(concrete); });
}

RecordView<Boolean<SemanticFamily, NonemptyTag>> QueryOptimizer::rewrite(RecordView<Boolean<SemanticFamily, NonemptyTag>> view)
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

RecordView<Numerical<SemanticFamily, DistanceTag>> QueryOptimizer::rewrite(RecordView<Numerical<SemanticFamily, DistanceTag>> view)
{
    using Data = urd::data<Numerical<SemanticFamily, DistanceTag>>;
    auto data = Data(optimize_argument<typename Data::Vertex>(view.get_lhs(), tyr::datalog::database::Evaluation::full),
                     optimize_argument<typename Data::Edge>(view.get_mid(), tyr::datalog::database::Evaluation::bound),
                     optimize_argument<typename Data::Vertex>(view.get_rhs(), tyr::datalog::database::Evaluation::full));
    return runir::kr::dl::insert(*m_repository, data).first;
}

RecordView<Query<QueryMustPassTag>> QueryOptimizer::rewrite(RecordView<Query<QueryMustPassTag>> view)
{
    auto data = urd::data<Query<QueryMustPassTag>>(view.data().keys,
                                                   optimize_root(view.get_sources(), std::nullopt).index(),
                                                   optimize_root(view.get_edges(), std::nullopt, tyr::datalog::database::Evaluation::bound).index(),
                                                   optimize_root(view.get_targets(), std::nullopt).index(),
                                                   optimize_root(view.get_probes(), std::nullopt).index());
    return runir::kr::dl::insert(*m_repository, data).first;
}

QueryView<> QueryOptimizer::optimize_root(QueryView<> root, const std::optional<Labels>& required, tyr::datalog::database::Evaluation evaluation)
{
    // Keyed as asked: no required labels (every column) and none required (no column) are different queries.
    const auto key = std::tuple { root, required, evaluation };
    if (const auto it = m_optimized.find(key); it != m_optimized.end())
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
    const auto result = lower(plan, leaves, *m_repository).front();
    m_optimized.emplace(key, result);
    return result;
}

}  // namespace runir::kr::dl
