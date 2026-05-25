#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_HPP_

#include "runir/datasets/task_class.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <cstddef>
#include <memory>
#include <tyr/planning/algorithms/brfs.hpp>
#include <tyr/planning/algorithms/iw.hpp>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/successor_generator.hpp>
#include <vector>

namespace runir::kr::ps::ext
{

enum class ModuleExecutionStatus
{
    SUCCESS,
    FAILURE,
    NO_APPLICABLE_ACTION,
    MALFORMED_CALL,
    SEARCH_FAILURE,
    OUT_OF_TIME,
    OUT_OF_STATES,
    LOAD_LIMIT_REACHED,
    STEP_LIMIT_REACHED,
};

template<tyr::planning::TaskKind Kind>
struct ModuleExecutionOptions
{
    tyr::planning::brfs::Options<Kind> brfs_options;
    tyr::planning::iw::Options<Kind> iw_options;
    tyr::uint_t max_arity = 0;
    std::size_t max_load_steps = 1024;
    std::size_t max_steps = 1024;
};

template<tyr::planning::TaskKind Kind>
struct ModuleExecutionResults
{
    ModuleExecutionStatus status = ModuleExecutionStatus::SUCCESS;
    tyr::planning::StateView<Kind> state;
    ModuleView module;
    MemoryStateView memory_state;
    std::size_t num_steps = 0;
    std::size_t call_depth = 0;

    bool is_successful() const noexcept { return status == ModuleExecutionStatus::SUCCESS; }
};

template<tyr::planning::TaskKind Kind>
auto execute_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                      ModuleView entry_module,
                      const std::vector<ModuleView>& modules,
                      const ModuleExecutionOptions<Kind>& options = ModuleExecutionOptions<Kind>()) -> ModuleExecutionResults<Kind>;

template<tyr::planning::TaskKind Kind>
auto execute_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                      ModuleView entry_module,
                      const ModuleExecutionOptions<Kind>& options = ModuleExecutionOptions<Kind>()) -> ModuleExecutionResults<Kind>;

template<tyr::planning::TaskKind Kind>
auto execute_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                      ModuleProgramView program,
                      const ModuleExecutionOptions<Kind>& options = ModuleExecutionOptions<Kind>()) -> ModuleExecutionResults<Kind>;

}  // namespace runir::kr::ps::ext

#include "runir/kr/ps/ext/module_program_executor_impl.hpp"

#endif
