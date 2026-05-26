#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_PROOF_GRAPH_HPP_

#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <cstddef>
#include <limits>
#include <optional>
#include <tuple>
#include <tyr/formalism/planning/ground_action_index.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::ps::ext
{

template<tyr::planning::TaskKind Kind>
struct ModuleProgramProofVertexLabel
{
    tyr::planning::StateView<Kind> state;
    ModuleView module_;
    MemoryStateView memory_state;
    ConstRepositoryPtr repository_owner;
    std::size_t call_depth = 0;
    tyr::float_t goal_distance = std::numeric_limits<tyr::float_t>::infinity();
    bool is_initial = false;
    bool is_goal = false;
    bool is_alive = false;
    bool is_unsolvable = false;

    ModuleProgramProofVertexLabel(tyr::planning::StateView<Kind> state_,
                                  ModuleView module__,
                                  MemoryStateView memory_state_,
                                  ConstRepositoryPtr repository_owner_,
                                  std::size_t call_depth_,
                                  tyr::float_t goal_distance_) noexcept :
        state(std::move(state_)),
        module_(module__),
        memory_state(memory_state_),
        repository_owner(std::move(repository_owner_)),
        call_depth(call_depth_),
        goal_distance(goal_distance_)
    {
    }

    auto identifying_members() const noexcept
    {
        return std::tie(state, module_, memory_state, call_depth, goal_distance, is_initial, is_goal, is_alive, is_unsolvable);
    }
};

struct ModuleProgramProofEdgeLabel
{
    std::optional<tyr::Index<tyr::formalism::planning::GroundAction>> action = std::nullopt;
    tyr::float_t cost = 0;

    auto identifying_members() const noexcept { return std::tie(action, cost); }
};

template<tyr::planning::TaskKind Kind>
using ModuleProgramProofGraphBuilder = graphs::StaticGraphBuilder<ModuleProgramProofVertexLabel<Kind>, ModuleProgramProofEdgeLabel>;

template<tyr::planning::TaskKind Kind>
using ModuleProgramProofGraph = graphs::StaticGraph<ModuleProgramProofVertexLabel<Kind>, ModuleProgramProofEdgeLabel>;

}  // namespace runir::kr::ps::ext

#endif
