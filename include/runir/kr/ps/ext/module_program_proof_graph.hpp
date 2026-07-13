#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_PROOF_GRAPH_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/ext/rule_variant_index.hpp"

#include <cista/containers/optional.h>
#include <limits>
#include <optional>
#include <tuple>
#include <tyr/formalism/planning/ground_action_index.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/planning/declarations.hpp>
#include <utility>

namespace runir::kr::ps::ext
{

template<tyr::planning::TaskKind Kind>
struct ModuleProgramProofVertexLabel
{
    ygg::Index<ExecutionState<Kind>> execution_state;
    ygg::float_t goal_distance = std::numeric_limits<ygg::float_t>::infinity();
    bool is_initial = false;
    bool is_goal = false;
    bool is_alive = false;
    bool is_unsolvable = false;

    auto identifying_members() const noexcept { return std::tie(execution_state, goal_distance, is_initial, is_goal, is_alive, is_unsolvable); }
};

struct ModuleProgramProofStateTransition
{
    ygg::Index<tyr::formalism::planning::GroundAction> action;
    ygg::float_t cost = 0;

    auto identifying_members() const noexcept { return std::tie(action, cost); }
};

struct ModuleProgramProofEdgeLabel
{
    ::cista::optional<ModuleProgramProofStateTransition> state_transition;
    ::cista::optional<ygg::Index<RuleVariant>> rule;

    auto identifying_members() const noexcept { return std::tie(state_transition, rule); }
};

template<tyr::planning::TaskKind Kind>
class ModuleProgramProofVertexLabelView
{
private:
    ModuleProgramProofVertexLabel<Kind> m_label;
    ExecutionRepositoryPtr<Kind> m_repository;

public:
    ModuleProgramProofVertexLabelView(ModuleProgramProofVertexLabel<Kind> label, ExecutionRepositoryPtr<Kind> repository) noexcept :
        m_label(std::move(label)),
        m_repository(std::move(repository))
    {
    }

    const auto& get_data() const noexcept { return m_label; }
    auto get_execution_state() const noexcept { return ExecutionStateView<Kind>(m_label.execution_state, m_repository); }
    auto get_state() const { return get_execution_state().get_state(); }
    auto get_goal_distance() const noexcept { return m_label.goal_distance; }
    bool is_initial() const noexcept { return m_label.is_initial; }
    bool is_goal() const noexcept { return m_label.is_goal; }
    bool is_alive() const noexcept { return m_label.is_alive; }
    bool is_unsolvable() const noexcept { return m_label.is_unsolvable; }
};

template<tyr::planning::TaskKind Kind>
class ModuleProgramProofEdgeLabelView
{
private:
    ModuleProgramProofEdgeLabel m_label;
    ExecutionRepositoryPtr<Kind> m_repository;

public:
    ModuleProgramProofEdgeLabelView(ModuleProgramProofEdgeLabel label, ExecutionRepositoryPtr<Kind> repository) noexcept :
        m_label(std::move(label)),
        m_repository(std::move(repository))
    {
    }

    const auto& get_data() const noexcept { return m_label; }

    auto get_state_transition() const -> std::optional<datasets::StateGraphEdgeLabel>
    {
        if (!m_label.state_transition)
            return std::nullopt;

        const auto& transition = *m_label.state_transition;
        auto& action_repository = *m_repository->get_task_context().search_context->task->get_repository();
        return datasets::StateGraphEdgeLabel { ygg::make_view(transition.action, action_repository), transition.cost };
    }

    auto get_rule() const -> std::optional<RuleVariantView>
    {
        if (!m_label.rule)
            return std::nullopt;
        return m_repository->get_rule(*m_label.rule);
    }
};

template<tyr::planning::TaskKind Kind>
using ModuleProgramProofGraphBuilder = graphs::StaticGraphBuilder<ModuleProgramProofVertexLabel<Kind>, ModuleProgramProofEdgeLabel>;

template<tyr::planning::TaskKind Kind>
class ModuleProgramProofGraph : public graphs::StaticGraph<ModuleProgramProofVertexLabel<Kind>, ModuleProgramProofEdgeLabel>
{
private:
    using Base = graphs::StaticGraph<ModuleProgramProofVertexLabel<Kind>, ModuleProgramProofEdgeLabel>;

    ExecutionRepositoryPtr<Kind> m_repository;

public:
    ModuleProgramProofGraph(ModuleProgramProofGraphBuilder<Kind>&& builder, ExecutionRepositoryPtr<Kind> repository) :
        Base(std::move(builder)),
        m_repository(std::move(repository))
    {
    }

    const auto& get_repository() const noexcept { return m_repository; }

    auto get_vertex_view(graphs::VertexIndex vertex) const
    {
        return ModuleProgramProofVertexLabelView<Kind>(Base::get_vertex(vertex).get_property(), m_repository);
    }

    auto get_edge_view(graphs::EdgeIndex edge) const { return ModuleProgramProofEdgeLabelView<Kind>(Base::get_edge(edge).get_property(), m_repository); }
};

}  // namespace runir::kr::ps::ext

namespace runir::graphs
{

template<tyr::planning::TaskKind Kind>
struct GraphTraits<runir::kr::ps::ext::ModuleProgramProofGraph<Kind>> :
    GraphTraits<StaticGraph<runir::kr::ps::ext::ModuleProgramProofVertexLabel<Kind>, runir::kr::ps::ext::ModuleProgramProofEdgeLabel>>
{
};

}  // namespace runir::graphs

#endif
