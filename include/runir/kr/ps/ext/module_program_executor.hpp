#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_HPP_

#include "runir/datasets/task_class.hpp"
#include "runir/kr/ps/ext/module_program_executor_data.hpp"
#include "runir/kr/ps/ext/repository.hpp"

namespace runir::kr::ps::ext
{

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
