#ifndef RUNIR_KR_GP_POLICY_EXECUTOR_IMPL_HPP_
#define RUNIR_KR_GP_POLICY_EXECUTOR_IMPL_HPP_

#include "runir/graphs/algorithms.hpp"
#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/gp/dl/evaluation_context.hpp"
#include "runir/kr/gp/policy_executor.hpp"

#include <tyr/planning/algorithms/brfs/event_handler.hpp>
#include <tyr/planning/algorithms/iw.hpp>

#include <algorithm>
#include <chrono>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

namespace runir::kr::gp
{
namespace detail
{

template<typename Graph>
class CycleVisitor : public graphs::bgl::TraversalVisitor<Graph>
{
private:

    const Graph& m_graph;
    tyr::UnorderedMap<graphs::VertexIndex, graphs::VertexIndex> m_parent;
    graphs::VertexIndexList m_cycle;

public:
    explicit CycleVisitor(const Graph& graph) : m_graph(graph) {}

    void tree_edge(graphs::EdgeIndex edge) override { m_parent[m_graph.get_target(edge)] = m_graph.get_source(edge); }

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

template<typename Graph>
auto find_cycle(const Graph& graph) -> graphs::VertexIndexList
{
    auto visitor = CycleVisitor<Graph>(graph);
    auto sources = graphs::VertexIndexList {};
    sources.reserve(graph.get_num_vertices());
    for (auto vertex : graph.get_vertex_indices())
        sources.push_back(vertex);

    graphs::algorithms::depth_first_visit(graph, sources, visitor);
    return visitor.get_cycle();
}

template<tyr::planning::TaskKind Kind>
class NeverGoalStrategy : public tyr::planning::GoalStrategy<Kind>
{
public:
    bool is_static_goal_satisfied(const tyr::planning::Task<Kind>& task) override
    {
        static_cast<void>(task);
        return true;
    }

    bool is_dynamic_goal_satisfied(const tyr::planning::StateView<Kind>& seed_state, const tyr::planning::StateView<Kind>& state) override
    {
        static_cast<void>(seed_state);
        static_cast<void>(state);
        return false;
    }
};



}  // namespace detail

template<tyr::planning::TaskKind Kind>
class PolicyGoalStrategy : public tyr::planning::GoalStrategy<Kind>
{
private:
    PolicyView m_policy;
    kr::dl::semantics::Builder m_dl_builder;
    kr::dl::semantics::DenotationRepositoryFactory m_dl_denotation_repository_factory;
    kr::dl::semantics::DenotationRepository m_dl_denotation_repository;

public:
    explicit PolicyGoalStrategy(PolicyView policy) :
        m_policy(policy),
        m_dl_builder(),
        m_dl_denotation_repository_factory(),
        m_dl_denotation_repository(m_dl_denotation_repository_factory.create())
    {
    }

    bool is_static_goal_satisfied(const tyr::planning::Task<Kind>& task) override
    {
        static_cast<void>(task);
        return true;
    }

    bool is_dynamic_goal_satisfied(const tyr::planning::StateView<Kind>& seed_state, const tyr::planning::StateView<Kind>& state) override
    {
        if (seed_state.get_index() == state.get_index())
            return false;

        auto context = dl::EvaluationContext(seed_state, state, m_dl_builder, m_dl_denotation_repository);
        return m_policy.is_compatible_with(context);
    }
};

namespace detail
{

template<tyr::planning::TaskKind Kind, typename GetOrCreateVertex>
class PolicyProofEventHandler : public tyr::planning::brfs::EventHandler<Kind>
{
private:
    graphs::VertexIndex m_source;
    tyr::planning::Node<Kind> m_source_node;
    PolicyGoalStrategy<Kind>& m_policy_goal_strategy;
    datasets::AnnotatedStateGraphBuilder<Kind>& m_builder;
    tyr::UnorderedSet<std::pair<graphs::VertexIndex, graphs::VertexIndex>>& m_policy_edges;
    graphs::VertexIndexList& m_open;
    const tyr::UnorderedSet<graphs::VertexIndex>& m_explored;
    GetOrCreateVertex& m_get_or_create_vertex;
    tyr::planning::Statistics m_statistics;
    bool m_found_compatible_transition = false;

public:
    PolicyProofEventHandler(graphs::VertexIndex source,
                            tyr::planning::Node<Kind> source_node,
                            PolicyGoalStrategy<Kind>& policy_goal_strategy,
                            datasets::AnnotatedStateGraphBuilder<Kind>& builder,
                            tyr::UnorderedSet<std::pair<graphs::VertexIndex, graphs::VertexIndex>>& policy_edges,
                            graphs::VertexIndexList& open,
                            const tyr::UnorderedSet<graphs::VertexIndex>& explored,
                            GetOrCreateVertex& get_or_create_vertex) :
        m_source(source),
        m_source_node(std::move(source_node)),
        m_policy_goal_strategy(policy_goal_strategy),
        m_builder(builder),
        m_policy_edges(policy_edges),
        m_open(open),
        m_explored(explored),
        m_get_or_create_vertex(get_or_create_vertex),
        m_statistics()
    {
    }

    bool found_compatible_transition() const noexcept { return m_found_compatible_transition; }

    void on_expand_node(const tyr::planning::Node<Kind>& node) override
    {
        static_cast<void>(node);
        m_statistics.increment_num_expanded();
    }

    void on_expand_goal_node(const tyr::planning::Node<Kind>& node) override { static_cast<void>(node); }

    void on_generate_node(const tyr::planning::Node<Kind>& source_node, const tyr::planning::LabeledNode<Kind>& labeled_succ_node) override
    {
        static_cast<void>(source_node);
        m_statistics.increment_num_generated();

        if (!m_policy_goal_strategy.is_dynamic_goal_satisfied(m_source_node.get_state(), labeled_succ_node.node.get_state()))
            return;

        m_found_compatible_transition = true;

        const auto target = m_get_or_create_vertex(labeled_succ_node.node);
        if (m_policy_edges.insert({ m_source, target }).second)
            m_builder.add_directed_edge(m_source, target, datasets::StateGraphEdgeLabel { labeled_succ_node.label, labeled_succ_node.node.get_metric() });

        if (!m_explored.contains(target))
            m_open.push_back(target);
    }

    void on_prune_node(const tyr::planning::Node<Kind>& node) override
    {
        static_cast<void>(node);
        m_statistics.increment_num_pruned();
    }

    void on_prune_node(const tyr::planning::Node<Kind>& source_node, const tyr::planning::LabeledNode<Kind>& labeled_succ_node) override
    {
        static_cast<void>(source_node);
        static_cast<void>(labeled_succ_node);
        m_statistics.increment_num_pruned();
    }

    void on_start_search(const tyr::planning::Node<Kind>& node) override
    {
        static_cast<void>(node);
        m_statistics = tyr::planning::Statistics();
        m_statistics.set_search_start_time_point(std::chrono::high_resolution_clock::now());
    }

    void on_finish_layer(uint_t layer) override { static_cast<void>(layer); }

    void on_end_search(tyr::planning::SearchStatus status) override
    {
        static_cast<void>(status);
        m_statistics.set_search_end_time_point(std::chrono::high_resolution_clock::now());
    }

    void on_solved(const tyr::planning::Plan<Kind>& plan) override { static_cast<void>(plan); }

    const tyr::planning::Statistics& get_search_statistics() const override { return m_statistics; }
    const tyr::planning::Statistics& get_statistics() const override { return m_statistics; }
};

}  // namespace detail

template<tyr::planning::TaskKind Kind>
auto prove_solution(const datasets::TaskSearchContext<Kind>& context, PolicyView policy, const PolicySearchOptions<Kind>& options) -> PolicyProofResults<Kind>
{
    auto result = PolicyProofResults<Kind> {};
    auto builder = datasets::AnnotatedStateGraphBuilder<Kind> {};
    auto state_to_vertex = tyr::UnorderedMap<tyr::planning::StateView<Kind>, graphs::VertexIndex> {};
    auto vertex_to_node = tyr::UnorderedMap<graphs::VertexIndex, tyr::planning::Node<Kind>> {};
    auto policy_edges = tyr::UnorderedSet<std::pair<graphs::VertexIndex, graphs::VertexIndex>> {};
    auto task_goal_strategy = tyr::planning::ConjunctiveGoalStrategy<Kind>(*context.task);
    auto policy_goal_strategy = PolicyGoalStrategy<Kind>(policy);
    const auto static_goal_satisfied = task_goal_strategy.is_static_goal_satisfied(*context.task);
    const auto initial_node = context.successor_generator->get_initial_node();
    const auto initial_state = initial_node.get_state();

    auto is_task_goal = [&](const tyr::planning::StateView<Kind>& state)
    {
        return static_goal_satisfied && task_goal_strategy.is_dynamic_goal_satisfied(initial_state, state);
    };

    auto make_label = [&](const tyr::planning::StateView<Kind>& state)
    {
        const auto goal = is_task_goal(state);
        return datasets::AnnotatedStateGraphVertexLabel<Kind> { state,
                                                                goal ? tyr::float_t(0) : std::numeric_limits<tyr::float_t>::infinity(),
                                                                tyr::EqualTo<tyr::planning::StateView<Kind>> {}(state, initial_state),
                                                                goal,
                                                                true,
                                                                false };
    };

    auto get_or_create_vertex = [&](const tyr::planning::Node<Kind>& node)
    {
        const auto state = node.get_state();
        if (const auto it = state_to_vertex.find(state); it != state_to_vertex.end())
            return it->second;

        const auto vertex = builder.add_vertex(make_label(state));
        state_to_vertex.emplace(state, vertex);
        vertex_to_node.emplace(vertex, node);

        return vertex;
    };


    auto open = graphs::VertexIndexList {};
    auto explored = tyr::UnorderedSet<graphs::VertexIndex> {};

    auto finish = [&](PolicyProofStatus status)
    {
        result.status = status;
        result.graph = datasets::StaticAnnotatedStateGraph<Kind>(std::move(builder));
        return std::move(result);
    };

    open.push_back(get_or_create_vertex(initial_node));

    while (!open.empty())
    {
        const auto source = open.back();
        open.pop_back();

        if (!explored.insert(source).second)
            continue;

        const auto source_node = vertex_to_node.at(source);
        const auto& source_label = builder.get_vertex(source).get_property();
        if (source_label.is_goal)
            continue;

        auto brfs_solver = tyr::planning::brfs::Solver<Kind> { context.task, context.successor_generator, options.brfs_options };
        auto iw_options = options.iw_options;
        auto event_handler = std::make_shared<detail::PolicyProofEventHandler<Kind, decltype(get_or_create_vertex)>>(source,
                                                                                                                     source_node,
                                                                                                                     policy_goal_strategy,
                                                                                                                     builder,
                                                                                                                     policy_edges,
                                                                                                                     open,
                                                                                                                     explored,
                                                                                                                     get_or_create_vertex);
        brfs_solver.options.event_handler = event_handler;
        iw_options.start_node = source_node;
        iw_options.goal_strategy = std::make_shared<detail::NeverGoalStrategy<Kind>>();

        const auto search_result = tyr::planning::iw::find_solution(brfs_solver, options.max_arity, iw_options);
        switch (search_result.status)
        {
            case tyr::planning::SearchStatus::EXHAUSTED:
                break;
            case tyr::planning::SearchStatus::OUT_OF_TIME:
                return finish(PolicyProofStatus::OUT_OF_TIME);
            case tyr::planning::SearchStatus::OUT_OF_STATES:
            case tyr::planning::SearchStatus::OUT_OF_MEMORY:
                return finish(PolicyProofStatus::OUT_OF_STATES);
            case tyr::planning::SearchStatus::FAILED:
            case tyr::planning::SearchStatus::UNSOLVABLE:
            case tyr::planning::SearchStatus::IN_PROGRESS:
            case tyr::planning::SearchStatus::SOLVED:
                return finish(PolicyProofStatus::FAILURE);
        }

        if (!event_handler->found_compatible_transition())
            result.open_states.push_back(source);
    }

    auto graph = datasets::StaticAnnotatedStateGraph<Kind>(std::move(builder));
    result.cycle = detail::find_cycle(graph);
    result.graph = std::move(graph);

    if (!result.open_states.empty() || !result.cycle.empty())
    {
        result.status = PolicyProofStatus::FAILURE;
        return std::move(result);
    }

    result.status = PolicyProofStatus::SUCCESS;
    return std::move(result);
}

template<tyr::planning::TaskKind Kind>
auto find_solution(const datasets::TaskSearchContext<Kind>& context,
                   PolicyView policy,
                   const PolicySearchOptions<Kind>& options) -> tyr::planning::SearchResult<Kind>
{
    auto brfs_solver = tyr::planning::brfs::Solver<Kind> { context.task, context.successor_generator, options.brfs_options };
    auto iw_solver = tyr::planning::iw::Solver<Kind> { std::move(brfs_solver), options.max_arity, options.iw_options };
    auto siw_options = options.siw_options;

    if (!siw_options.subgoal_strategy)
        siw_options.subgoal_strategy = std::make_shared<PolicyGoalStrategy<Kind>>(policy);
    if (!siw_options.goal_strategy)
        siw_options.goal_strategy = tyr::planning::ConjunctiveGoalStrategy<Kind>::create(*context.task);

    return tyr::planning::siw::find_solution(iw_solver, siw_options);
}

}  // namespace runir::kr::gp

#endif
