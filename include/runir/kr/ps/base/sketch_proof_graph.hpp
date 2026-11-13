#ifndef RUNIR_KR_PS_BASE_SKETCH_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_PROOF_GRAPH_HPP_

#include "runir/datasets/graph_types.hpp"
#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/rule_view.hpp"

#include <tuple>
#include <tyr/formalism/declarations.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_repository.hpp>
#include <urd/repr/graphs/csr_graph.hpp>
#include <utility>
#include <yggdrasil/formalism/row_view.hpp>

namespace runir::kr::ps::base
{

template<tyr::TaskKind Kind>
struct SketchProofVertexLabel : urd::compared_by_identity<SketchProofVertexLabel<Kind>>
{
    tyr::planning::PackedStateView<Kind> state;
    bool is_initial = false;
    bool is_goal = false;
    bool is_alive = false;
    bool is_unsolvable = false;

    SketchProofVertexLabel(tyr::planning::PackedStateView<Kind> state_, bool is_initial_, bool is_goal_, bool is_alive_, bool is_unsolvable_) noexcept :
        state(std::move(state_)),
        is_initial(is_initial_),
        is_goal(is_goal_),
        is_alive(is_alive_),
        is_unsolvable(is_unsolvable_)
    {
    }

    static constexpr auto cista_members() noexcept
    {
        return urd::Members<&SketchProofVertexLabel::state,
                            &SketchProofVertexLabel::is_initial,
                            &SketchProofVertexLabel::is_goal,
                            &SketchProofVertexLabel::is_alive,
                            &SketchProofVertexLabel::is_unsolvable> {};
    }
    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&SketchProofVertexLabel::state,
                            &SketchProofVertexLabel::is_initial,
                            &SketchProofVertexLabel::is_goal,
                            &SketchProofVertexLabel::is_alive,
                            &SketchProofVertexLabel::is_unsolvable> {};
    }
};

struct SketchProofEdgeLabel : urd::compared_by_identity<SketchProofEdgeLabel>
{
    tyr::formalism::planning::ActionRowView action;
    RuleView rule;

    SketchProofEdgeLabel(tyr::formalism::planning::ActionRowView action_, RuleView rule_) noexcept : action(action_), rule(rule_) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&SketchProofEdgeLabel::action, &SketchProofEdgeLabel::rule> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&SketchProofEdgeLabel::action, &SketchProofEdgeLabel::rule> {}; }
};

/// The states a sketch's proof search reached and the transitions it took, each labelled by its action and rule: a
/// frozen graph, each state labelled too.
template<tyr::TaskKind Kind>
using SketchProofGraph = urd::Store<urd::graph<VertexIndex, SketchProofEdgeLabel, SketchProofVertexLabel<Kind>>, urd::frozen>;
// The concepts it is bound and searched by (urd/graphs.hpp): a renamed member is a compile error, not a lost feature.
static_assert(urd::incidence_graph<SketchProofGraph<tyr::GroundTag>> && urd::edge_list_graph<SketchProofGraph<tyr::LiftedTag>>);

}  // namespace runir::kr::ps::base

#endif
