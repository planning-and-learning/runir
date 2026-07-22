#ifndef RUNIR_KR_PS_BASE_SKETCH_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_PROOF_GRAPH_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_view.hpp"

#include <tuple>
#include <tyr/planning/declarations.hpp>
#include <utility>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::kr::ps::base
{

template<tyr::planning::TaskKind Kind>
struct SketchProofVertexLabel : ygg::comparison::Mixin<SketchProofVertexLabel<Kind>>
{
    tyr::planning::StateView<Kind> state;
    bool is_initial = false;
    bool is_goal = false;
    bool is_alive = false;
    bool is_unsolvable = false;

    SketchProofVertexLabel(tyr::planning::StateView<Kind> state_, bool is_initial_, bool is_goal_, bool is_alive_, bool is_unsolvable_) noexcept :
        state(std::move(state_)),
        is_initial(is_initial_),
        is_goal(is_goal_),
        is_alive(is_alive_),
        is_unsolvable(is_unsolvable_)
    {
    }

    auto cista_members() noexcept { return std::tie(state, is_initial, is_goal, is_alive, is_unsolvable); }
    auto identifying_members() const noexcept { return std::tie(state, is_initial, is_goal, is_alive, is_unsolvable); }
};

struct SketchProofEdgeLabel : ygg::comparison::Mixin<SketchProofEdgeLabel>
{
    runir::datasets::StateGraphEdgeLabel transition;
    RuleView rule;

    SketchProofEdgeLabel(runir::datasets::StateGraphEdgeLabel transition_, RuleView rule_) noexcept : transition(std::move(transition_)), rule(rule_) {}

    auto cista_members() noexcept { return std::tie(transition, rule); }
    auto identifying_members() const noexcept { return std::tie(transition, rule); }
};

template<tyr::planning::TaskKind Kind>
using SketchProofGraphBuilder = graphs::StaticGraphBuilder<SketchProofVertexLabel<Kind>, SketchProofEdgeLabel>;

template<tyr::planning::TaskKind Kind>
using SketchProofGraph = graphs::StaticGraph<SketchProofVertexLabel<Kind>, SketchProofEdgeLabel>;

}  // namespace runir::kr::ps::base

#endif
