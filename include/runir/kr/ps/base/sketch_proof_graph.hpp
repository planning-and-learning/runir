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

namespace runir::kr::ps::base
{

template<tyr::planning::TaskKind Kind>
struct SketchProofVertexLabel
{
    tyr::planning::StateView<Kind> state;
    bool is_initial = false;
    bool is_goal = false;
    bool is_alive = false;
    bool is_unsolvable = false;

    auto identifying_members() const noexcept { return std::tie(state, is_initial, is_goal, is_alive, is_unsolvable); }
};

struct SketchProofEdgeLabel
{
    runir::datasets::StateGraphEdgeLabel transition;
    RuleView rule;

    SketchProofEdgeLabel(runir::datasets::StateGraphEdgeLabel transition_, RuleView rule_) noexcept : transition(std::move(transition_)), rule(rule_) {}

    auto identifying_members() const noexcept { return std::tie(transition, rule); }
};

template<tyr::planning::TaskKind Kind>
using SketchProofGraphBuilder = graphs::StaticGraphBuilder<SketchProofVertexLabel<Kind>, SketchProofEdgeLabel>;

template<tyr::planning::TaskKind Kind>
using SketchProofGraph = graphs::StaticGraph<SketchProofVertexLabel<Kind>, SketchProofEdgeLabel>;

}  // namespace runir::kr::ps::base

#endif
