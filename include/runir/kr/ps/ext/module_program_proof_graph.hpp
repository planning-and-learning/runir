#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_PROOF_GRAPH_HPP_

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

template<tyr::planning::TaskKind Kind>
struct ModuleProgramProofVertexLabel : ygg::comparison::Mixin<ModuleProgramProofVertexLabel<Kind>>
{
    ExecutionStateView<Kind> execution_state;
    bool is_initial = false;
    bool is_goal = false;
    bool is_alive = false;
    bool is_unsolvable = false;

    ModuleProgramProofVertexLabel(ExecutionStateView<Kind> execution_state_, bool is_initial_, bool is_goal_, bool is_alive_, bool is_unsolvable_) noexcept :
        execution_state(std::move(execution_state_)),
        is_initial(is_initial_),
        is_goal(is_goal_),
        is_alive(is_alive_),
        is_unsolvable(is_unsolvable_)
    {
    }

    auto cista_members() noexcept { return std::tie(execution_state, is_initial, is_goal, is_alive, is_unsolvable); }
    auto identifying_members() const noexcept { return std::make_tuple(execution_state, is_initial, is_goal, is_alive, is_unsolvable); }
};

struct ModuleProgramProofStateTransition : ygg::comparison::Mixin<ModuleProgramProofStateTransition>
{
    tyr::formalism::planning::GroundActionView action;
    ygg::float_t cost = 0;

    ModuleProgramProofStateTransition(tyr::formalism::planning::GroundActionView action_, ygg::float_t cost_) noexcept : action(action_), cost(cost_) {}

    auto cista_members() noexcept { return std::tie(action, cost); }
    auto identifying_members() const noexcept { return std::make_tuple(action, cost); }
};

struct ModuleProgramProofEdgeLabel : ygg::comparison::Mixin<ModuleProgramProofEdgeLabel>
{
    ::cista::optional<ModuleProgramProofStateTransition> state_transition;
    ::cista::optional<RuleVariantView> rule;

    ModuleProgramProofEdgeLabel() = default;
    ModuleProgramProofEdgeLabel(::cista::optional<ModuleProgramProofStateTransition> state_transition_, ::cista::optional<RuleVariantView> rule_) :
        state_transition(std::move(state_transition_)),
        rule(std::move(rule_))
    {
    }

    auto cista_members() noexcept { return std::tie(state_transition, rule); }
    auto identifying_members() const noexcept { return std::tie(state_transition, rule); }
};

template<tyr::planning::TaskKind Kind>
using ModuleProgramProofGraphBuilder = graphs::StaticGraphBuilder<ModuleProgramProofVertexLabel<Kind>, ModuleProgramProofEdgeLabel>;

template<tyr::planning::TaskKind Kind>
using ModuleProgramProofGraph = graphs::StaticGraph<ModuleProgramProofVertexLabel<Kind>, ModuleProgramProofEdgeLabel>;

}  // namespace runir::kr::ps::ext

#endif
