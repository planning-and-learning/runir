#ifndef RUNIR_KR_PS_EXT_PROGRAM_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_EXT_PROGRAM_PROOF_GRAPH_HPP_

#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"

#include <cista/containers/optional.h>
#include <tuple>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/planning/declarations.hpp>
#include <utility>
#include <yggdrasil/semantics/comparison.hpp>
#include <yggdrasil/serialization/cista_equal_to.hpp>
#include <yggdrasil/serialization/cista_hash.hpp>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind>
struct ProgramProofVertexLabel : ygg::comparison::Mixin<ProgramProofVertexLabel<Kind>>
{
    ProgramStateView<Kind> program_state;
    bool is_initial = false;
    bool is_goal = false;
    bool is_alive = false;
    bool is_unsolvable = false;

    ProgramProofVertexLabel(ProgramStateView<Kind> program_state_, bool is_initial_, bool is_goal_, bool is_alive_, bool is_unsolvable_) noexcept :
        program_state(std::move(program_state_)),
        is_initial(is_initial_),
        is_goal(is_goal_),
        is_alive(is_alive_),
        is_unsolvable(is_unsolvable_)
    {
    }

    auto cista_members() noexcept { return std::tie(program_state, is_initial, is_goal, is_alive, is_unsolvable); }
    auto identifying_members() const noexcept { return std::make_tuple(program_state, is_initial, is_goal, is_alive, is_unsolvable); }
};

struct ProgramProofStateTransition : ygg::comparison::Mixin<ProgramProofStateTransition>
{
    tyr::formalism::planning::ActionBindingView action;
    ygg::float_t cost = 0;

    ProgramProofStateTransition(tyr::formalism::planning::ActionBindingView action_, ygg::float_t cost_) noexcept : action(action_), cost(cost_) {}

    auto cista_members() noexcept { return std::tie(action, cost); }
    auto identifying_members() const noexcept { return std::make_tuple(action, cost); }
};

struct ProgramProofEdgeLabel : ygg::comparison::Mixin<ProgramProofEdgeLabel>
{
    ::cista::optional<ProgramProofStateTransition> state_transition;
    ::cista::optional<RuleVariantView> rule;

    ProgramProofEdgeLabel() = default;
    ProgramProofEdgeLabel(::cista::optional<ProgramProofStateTransition> state_transition_, ::cista::optional<RuleVariantView> rule_) :
        state_transition(std::move(state_transition_)),
        rule(std::move(rule_))
    {
    }

    auto cista_members() noexcept { return std::tie(state_transition, rule); }
    auto identifying_members() const noexcept { return std::tie(state_transition, rule); }
};

template<tyr::TaskKind Kind>
using ProgramProofGraphBuilder = graphs::StaticGraphBuilder<ProgramProofVertexLabel<Kind>, ProgramProofEdgeLabel>;

template<tyr::TaskKind Kind>
using ProgramProofGraph = graphs::StaticGraph<ProgramProofVertexLabel<Kind>, ProgramProofEdgeLabel>;

}  // namespace runir::kr::ps::ext

#endif
