#include "runir/kr/gp/policy_executor.hpp"

#include "runir/graphs/algorithms.hpp"
#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/gp/dl/evaluation_context.hpp"

#include <algorithm>
#include <stdexcept>

namespace runir::kr::gp
{
namespace
{

template<tyr::planning::TaskKind Kind>
using AnnotatedStateGraphView = typename datasets::AnnotatedStateGraph<Kind>::ForwardGraphType;

template<tyr::planning::TaskKind Kind>
auto find_initial_vertex(const AnnotatedStateGraphView<Kind>& graph) -> graphs::VertexIndex
{
    for (auto vertex : graph.get_vertex_indices())
        if (graph.get_vertex(vertex).get_property().is_initial)
            return vertex;

    throw std::runtime_error("Cannot execute policy on state graph without an initial state.");
}

template<tyr::planning::TaskKind Kind>
auto is_policy_compatible_transition(PolicyView policy,
                                     const AnnotatedStateGraphView<Kind>& graph,
                                     graphs::EdgeIndex edge,
                                     kr::dl::semantics::Builder& dl_builder,
                                     kr::dl::semantics::DenotationRepository& dl_denotation_repository) -> bool
{
    const auto source = graph.get_source(edge);
    const auto target = graph.get_target(edge);
    auto context = dl::EvaluationContext(graph.get_vertex(source).get_property().state,
                                         graph.get_vertex(target).get_property().state,
                                         dl_builder,
                                         dl_denotation_repository);

    return policy.is_compatible_with(context);
}

template<tyr::planning::TaskKind Kind>
class CycleVisitor : public graphs::bgl::TraversalVisitor<datasets::DynamicAnnotatedStateGraph<Kind>>
{
private:
    using Graph = datasets::DynamicAnnotatedStateGraph<Kind>;

    const Graph& m_graph;
    tyr::UnorderedMap<graphs::VertexIndex, graphs::VertexIndex> m_parent;
    graphs::VertexIndexList m_cycle;

public:
    explicit CycleVisitor(const Graph& graph) : m_graph(graph) {}

    void tree_edge(graphs::EdgeIndex edge) override
    {
        m_parent[m_graph.get_target(edge)] = m_graph.get_source(edge);
    }

    void back_edge(graphs::EdgeIndex edge) override
    {
        if (!m_cycle.empty())
            return;

        const auto source = m_graph.get_source(edge);
        const auto target = m_graph.get_target(edge);

        m_cycle.push_back(target);
        for (auto vertex = source; vertex != target; vertex = m_parent.at(vertex))
            m_cycle.push_back(vertex);
        m_cycle.push_back(target);
        std::ranges::reverse(m_cycle);
    }

    auto get_cycle() const -> const graphs::VertexIndexList& { return m_cycle; }
};

template<tyr::planning::TaskKind Kind>
auto find_cycle(const datasets::DynamicAnnotatedStateGraph<Kind>& graph) -> graphs::VertexIndexList
{
    auto visitor = CycleVisitor<Kind>(graph);
    auto sources = graphs::VertexIndexList {};
    sources.reserve(graph.get_num_vertices());
    for (auto vertex : graph.get_vertex_indices())
        sources.push_back(vertex);

    graphs::algorithms::depth_first_visit(graph, sources, visitor);
    return visitor.get_cycle();
}

}  // namespace

template<tyr::planning::TaskKind Kind>
auto execute_policy(const datasets::AnnotatedStateGraph<Kind>& graph, PolicyView policy) -> PolicyExecutionResults<Kind>
{
    const auto& forward_graph = graph.get_forward_graph();
    const auto initial_vertex = find_initial_vertex<Kind>(forward_graph);

    auto result = PolicyExecutionResults<Kind> {};
    auto original_to_execution_vertex = tyr::UnorderedMap<graphs::VertexIndex, graphs::VertexIndex> {};
    auto execution_to_original_vertex = graphs::VertexIndexList {};

    auto get_or_create_execution_vertex = [&](graphs::VertexIndex vertex) {
        const auto it = original_to_execution_vertex.find(vertex);
        if (it != original_to_execution_vertex.end())
            return it->second;

        const auto execution_vertex = result.graph.add_vertex(forward_graph.get_vertex(vertex).get_property());
        original_to_execution_vertex.emplace(vertex, execution_vertex);

        if (execution_to_original_vertex.size() <= execution_vertex)
            execution_to_original_vertex.resize(execution_vertex + 1);
        execution_to_original_vertex[execution_vertex] = vertex;

        return execution_vertex;
    };

    auto dl_builder = kr::dl::semantics::Builder();
    auto dl_denotation_repository_factory = kr::dl::semantics::DenotationRepositoryFactory();
    auto dl_denotation_repository = dl_denotation_repository_factory.create();
    auto open = graphs::VertexIndexList {};
    auto explored = tyr::UnorderedSet<graphs::VertexIndex> {};

    open.push_back(initial_vertex);
    get_or_create_execution_vertex(initial_vertex);

    while (!open.empty())
    {
        const auto source = open.back();
        open.pop_back();

        if (!explored.insert(source).second)
            continue;

        auto has_compatible_transition = false;
        for (auto edge : forward_graph.get_out_edge_indices(source))
        {
            if (!is_policy_compatible_transition<Kind>(policy, forward_graph, edge, dl_builder, dl_denotation_repository))
                continue;

            has_compatible_transition = true;

            const auto target = forward_graph.get_target(edge);
            const auto execution_source = get_or_create_execution_vertex(source);
            const auto execution_target = get_or_create_execution_vertex(target);
            result.graph.add_directed_edge(execution_source, execution_target, forward_graph.get_edge(edge).get_property());

            if (forward_graph.get_vertex(target).get_property().is_unsolvable)
            {
                result.deadend_transitions.push_back(edge);
                continue;
            }

            if (!explored.contains(target))
                open.push_back(target);
        }

        const auto& label = forward_graph.get_vertex(source).get_property();
        if (!has_compatible_transition && label.is_alive && !label.is_goal)
            result.open_states.push_back(source);
    }

    for (auto vertex : find_cycle(result.graph))
        result.cycle.push_back(execution_to_original_vertex.at(vertex));

    if (!result.deadend_transitions.empty() || !result.open_states.empty() || !result.cycle.empty())
        result.status = PolicyExecutionStatus::FAILURE;

    return result;
}

template auto execute_policy<tyr::planning::GroundTag>(const datasets::AnnotatedStateGraph<tyr::planning::GroundTag>& graph,
                                                       PolicyView policy) -> PolicyExecutionResults<tyr::planning::GroundTag>;

template auto execute_policy<tyr::planning::LiftedTag>(const datasets::AnnotatedStateGraph<tyr::planning::LiftedTag>& graph,
                                                       PolicyView policy) -> PolicyExecutionResults<tyr::planning::LiftedTag>;

}  // namespace runir::kr::gp
