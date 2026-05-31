#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_HPP_

#include "runir/datasets/task_class.hpp"
#include "runir/graphs/declarations.hpp"
#include "runir/kr/ps/ext/module_program_proof_graph.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <tyr/planning/algorithms/brfs.hpp>
#include <tyr/planning/algorithms/iw.hpp>
#include <tyr/planning/declarations.hpp>
#include <vector>

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
    runir::datasets::TaskSearchContextPtr<Kind> context_owner;
    std::shared_ptr<ModuleProgramProofGraph<Kind>> graph;
    std::optional<tyr::planning::StateView<Kind>> final_state = std::nullopt;
    std::optional<tyr::planning::Plan<Kind>> plan = std::nullopt;
    runir::graphs::EdgeIndexList deadend_transitions;
    runir::graphs::VertexIndexList open_states;
    runir::graphs::VertexIndexList cycle;

    bool is_successful() const noexcept { return status == ModuleProgramProofStatus::SUCCESS; }
};

template<tyr::planning::TaskKind Kind>
auto prove_solution(runir::datasets::TaskSearchContextPtr<Kind> context,
                    ModuleProgramView program,
                    const ModuleProgramSearchOptions<Kind>& options = ModuleProgramSearchOptions<Kind>()) -> ModuleProgramProofResults<Kind>;

template<tyr::planning::TaskKind Kind>
auto find_solution(runir::datasets::TaskSearchContextPtr<Kind> context,
                   ModuleProgramView program,
                   const ModuleProgramSearchOptions<Kind>& options = ModuleProgramSearchOptions<Kind>()) -> ModuleProgramProofResults<Kind>;

}  // namespace runir::kr::ps::ext

#include "runir/kr/ps/ext/module_program_executor_impl.hpp"

#endif
