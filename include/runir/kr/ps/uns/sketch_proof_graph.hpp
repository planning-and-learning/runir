#ifndef RUNIR_KR_PS_UNS_SKETCH_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_UNS_SKETCH_PROOF_GRAPH_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/uns/repository.hpp"

#include <tuple>
#include <utility>
#include <tyr/planning/declarations.hpp>

namespace runir::kr::ps::uns
{

struct SketchProofEdgeLabel
{
    runir::datasets::StateGraphEdgeLabel transition;
    RuleView rule;

    SketchProofEdgeLabel(runir::datasets::StateGraphEdgeLabel transition_, RuleView rule_) noexcept :
        transition(std::move(transition_)),
        rule(rule_)
    {
    }

    auto identifying_members() const noexcept { return std::tie(transition, rule); }
};

template<tyr::planning::TaskKind Kind>
using SketchProofGraphBuilder = graphs::StaticGraphBuilder<runir::datasets::AnnotatedStateGraphVertexLabel<Kind>, SketchProofEdgeLabel>;

template<tyr::planning::TaskKind Kind>
using SketchProofGraph = graphs::StaticGraph<runir::datasets::AnnotatedStateGraphVertexLabel<Kind>, SketchProofEdgeLabel>;

}  // namespace runir::kr::ps::uns

#endif
