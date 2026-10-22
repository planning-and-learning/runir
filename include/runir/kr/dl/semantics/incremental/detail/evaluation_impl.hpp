#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATION_IMPL_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATION_IMPL_HPP_

#include "runir/kr/dl/semantics/incremental/detail/feature_node.hpp"
#include "runir/kr/dl/semantics/incremental/detail/query_node.hpp"
#include "runir/kr/dl/semantics/incremental/evaluation.hpp"

namespace runir::kr::dl::semantics::incremental
{

/// Defined out of class so that only the library destroys the evaluation nodes and their evaluators.
template<tyr::TaskKind Kind>
EvaluationGraph<Kind>::~EvaluationGraph() = default;

template<tyr::TaskKind Kind>
IncrementalEvaluation<Kind>::~IncrementalEvaluation() = default;

template<tyr::TaskKind Kind>
EvaluationGraph<Kind>::EvaluationGraph(EvaluationGraph&&) noexcept = default;

template<tyr::TaskKind Kind>
EvaluationGraph<Kind>& EvaluationGraph<Kind>::operator=(EvaluationGraph&&) noexcept = default;

template<tyr::TaskKind Kind>
size_t EvaluationGraph<Kind>::node_count() const noexcept
{
    return m_nodes.size();
}

template<tyr::TaskKind Kind>
template<CategoryTag Category>
BorrowedDenotationView<Category> EvaluationGraph<Kind>::result(EvaluationIndex<Category> index) const
{
    return node(index).get_result(repository());
}

template<tyr::TaskKind Kind>
const ygg::Builder<ygg::database::Relation>& EvaluationGraph<Kind>::result(QueryEvaluationIndex index) const
{
    return node(index).get_result();
}

template<tyr::TaskKind Kind>
template<ConceptOrRoleTag Category>
const DenotationDelta<Category>& EvaluationGraph<Kind>::change(EvaluationIndex<Category> index) const
{
    return node(index).get_delta();
}

template<tyr::TaskKind Kind>
const ygg::database::incremental::Delta& EvaluationGraph<Kind>::change(QueryEvaluationIndex index) const
{
    return node(index).get_delta();
}

template<tyr::TaskKind Kind>
template<ConceptOrRoleTag Category>
size_t EvaluationGraph<Kind>::cardinality(EvaluationIndex<Category> index) const
{
    return node(index).size();
}

template<tyr::TaskKind Kind>
template<CategoryTag Category>
EvaluationIndex<Category> EvaluationGraph<Kind>::get_index(ConstructorView<Category> expression) const
{
    if (const auto index = find_node(expression))
        return *index;
    throw std::invalid_argument("Incremental evaluation: expression is not prepared in this graph.");
}

template<tyr::TaskKind Kind>
QueryEvaluationIndex EvaluationGraph<Kind>::get_index(QueryView<> expression) const
{
    if (const auto index = find_node(expression))
        return *index;
    throw std::invalid_argument("Incremental evaluation: expression is not prepared in this graph.");
}

template<tyr::TaskKind Kind>
template<CategoryTag Category>
std::optional<EvaluationIndex<Category>> EvaluationGraph<Kind>::find_node(ConstructorView<Category> expression) const
{
    // ponytail: linear preparation/root lookup; add a lookup table only if large graphs make it costly.
    for (size_t i = 0; i < m_nodes.size(); ++i)
    {
        const auto* candidate = std::get_if<detail::FeatureNode<Kind, Category>>(&m_nodes[i]);
        if (candidate && candidate->get_expression() == expression)
            return EvaluationIndex<Category>(ygg::to_uint_t(i));
    }
    return std::nullopt;
}

template<tyr::TaskKind Kind>
std::optional<QueryEvaluationIndex> EvaluationGraph<Kind>::find_node(QueryView<> expression) const
{
    for (size_t i = 0; i < m_nodes.size(); ++i)
    {
        const auto* candidate = std::get_if<detail::QueryNode<Kind>>(&m_nodes[i]);
        if (candidate && candidate->get_expression() == expression)
            return QueryEvaluationIndex(ygg::to_uint_t(i));
    }
    return std::nullopt;
}

template<tyr::TaskKind Kind>
template<BooleanOrNumericalTag Category>
bool IncrementalEvaluation<Kind>::changed(EvaluationIndex<Category> index) const
{
    require_exact(ygg::uint_t(index));
    return m_graph.node(index).changed();
}

template<tyr::TaskKind Kind>
EvaluationGraph<Kind>::EvaluationGraph(const tyr::planning::Task<Kind>& task, std::span<const EvaluationRoot> roots) : m_task(&task)
{
    m_set_workspace.initialize(ygg::to_uint_t(task.get_task().get_num_objects()));
    for (const auto& root : roots)
        std::visit([&](auto expression) { prepare(expression); }, root);
}

template<tyr::TaskKind Kind>
template<CategoryTag Category>
EvaluationIndex<Category> EvaluationGraph<Kind>::prepare(ConstructorView<Category> expression)
{
    if (const auto existing = find_node(expression))
        return *existing;
    // Construct locally: preparing children may relocate m_nodes.
    auto builder = detail::GraphBuilder<Kind>(*this);
    auto prepared = detail::FeatureNode<Kind, Category>(expression, builder);
    const auto index = EvaluationIndex<Category>(ygg::to_uint_t(m_nodes.size()));
    m_nodes.emplace_back(std::move(prepared));
    return index;
}

template<tyr::TaskKind Kind>
QueryEvaluationIndex EvaluationGraph<Kind>::prepare(QueryView<> expression)
{
    if (const auto existing = find_node(expression))
        return *existing;
    auto builder = detail::GraphBuilder<Kind>(*this);
    auto prepared = detail::QueryNode<Kind>(expression, builder);
    const auto index = QueryEvaluationIndex(ygg::to_uint_t(m_nodes.size()));
    m_nodes.emplace_back(std::move(prepared));
    return index;
}

template<tyr::TaskKind Kind>
template<StateEvaluationContextConcept<Kind> Context>
void IncrementalEvaluation<Kind>::demand(ygg::uint_t index, Context& context, const Delta* delta, ygg::database::Workspace* workspace)
{
    if (m_nodes.at(index).epoch == m_epoch)
        return;
    std::visit(
        [&](auto& evaluator)
        {
            // Finish children before borrowing the shared operation workspace.
            auto children_exact = true;
            for (const auto child : evaluator.get_dependencies())
            {
                demand(child, context, delta, workspace);
                children_exact = children_exact && m_nodes[child].has_delta;
            }
            auto& node = m_nodes[index];
            const auto update = delta && node.epoch != 0 && node.epoch + 1 == m_epoch && children_exact;
            node = NodeState {};
            if (update)
                evaluator.update(m_graph, *delta, *workspace);
            else
                evaluator.initialize(m_graph, context);
            node = NodeState { m_epoch, update };
        },
        m_graph.m_nodes[index]);
}

}  // namespace runir::kr::dl::semantics::incremental

#endif
