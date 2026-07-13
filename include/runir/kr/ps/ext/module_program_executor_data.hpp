#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_DATA_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_DATA_HPP_

#include "runir/graphs/declarations.hpp"
#include "runir/kr/ps/ext/module_program_proof_graph.hpp"
#include "runir/kr/task_context.hpp"

#include <chrono>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <tyr/planning/declarations.hpp>

namespace runir::kr::ps::ext
{

enum class ModuleProgramProofStatus
{
    SUCCESS,
    FAILURE,
    OUT_OF_TIME,
    OUT_OF_STATES,
};

template<tyr::planning::TaskKind Kind>
struct ModuleProgramSearchOptions
{
    ygg::uint_t max_num_states = std::numeric_limits<ygg::uint_t>::max();
    std::optional<std::chrono::steady_clock::duration> max_time = std::nullopt;
    uint64_t random_seed = 0;
    bool shuffle_labeled_succ_nodes = false;
};

template<tyr::planning::TaskKind Kind>
struct ModuleProgramProofResults
{
    ModuleProgramProofStatus status = ModuleProgramProofStatus::SUCCESS;
    runir::kr::TaskContextPtr<Kind> task_context_owner;
    std::shared_ptr<ModuleProgramProofGraph<Kind>> graph;
    std::optional<tyr::planning::StateView<Kind>> final_state = std::nullopt;
    std::optional<tyr::planning::Plan<Kind>> plan = std::nullopt;
    runir::graphs::EdgeIndexList deadend_transitions;
    runir::graphs::VertexIndexList open_states;
    runir::graphs::VertexIndexList cycle;

    bool is_successful() const noexcept { return status == ModuleProgramProofStatus::SUCCESS; }
};

}  // namespace runir::kr::ps::ext

#endif
