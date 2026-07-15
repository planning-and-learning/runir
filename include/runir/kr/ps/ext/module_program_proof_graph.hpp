#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_PROOF_GRAPH_HPP_

#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"

#include <cista/containers/optional.h>
#include <tuple>
#include <tyr/formalism/planning/ground_action_index.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/planning/declarations.hpp>
#include <yggdrasil/serialization/cista_equal_to.hpp>
#include <yggdrasil/serialization/cista_hash.hpp>

namespace runir::kr::ps::ext
{

template<tyr::planning::TaskKind Kind>
struct ModuleProgramProofVertexLabel
{
    ExecutionStateView<Kind> execution_state;
    bool is_initial = false;
    bool is_goal = false;
    bool is_alive = false;
    bool is_unsolvable = false;

    auto identifying_members() const noexcept { return std::make_tuple(execution_state, is_initial, is_goal, is_alive, is_unsolvable); }
};

struct ModuleProgramProofStateTransition
{
    tyr::formalism::planning::GroundActionView action;
    ygg::float_t cost = 0;

    auto identifying_members() const noexcept { return std::make_tuple(action, cost); }
};

struct ModuleProgramProofEdgeLabel
{
    ::cista::optional<ModuleProgramProofStateTransition> state_transition;
    ::cista::optional<RuleVariantView> rule;

    auto identifying_members() const noexcept { return std::tie(state_transition, rule); }
};

template<tyr::planning::TaskKind Kind>
using ModuleProgramProofGraphBuilder = graphs::StaticGraphBuilder<ModuleProgramProofVertexLabel<Kind>, ModuleProgramProofEdgeLabel>;

template<tyr::planning::TaskKind Kind>
using ModuleProgramProofGraph = graphs::StaticGraph<ModuleProgramProofVertexLabel<Kind>, ModuleProgramProofEdgeLabel>;

}  // namespace runir::kr::ps::ext

#endif
