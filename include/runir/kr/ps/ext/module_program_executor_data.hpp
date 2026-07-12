#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_DATA_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_DATA_HPP_

#include "runir/graphs/declarations.hpp"
#include "runir/kr/ps/ext/module_program_proof_graph.hpp"
#include "runir/kr/task_context.hpp"

#include <memory>
#include <optional>
#include <tyr/planning/algorithms/brfs.hpp>
#include <tyr/planning/algorithms/iw.hpp>
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
    tyr::planning::brfs::Options<Kind> brfs_options;
    tyr::planning::iw::Options<Kind> iw_options;
    ygg::uint_t max_arity = 0;
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
