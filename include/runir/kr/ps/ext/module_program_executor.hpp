#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_HPP_

#include "runir/kr/ps/ext/module_program_executor_data.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/task_context.hpp"

namespace runir::kr::ps::ext
{

template<tyr::planning::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context,
                   ModuleProgramView program,
                   const ModuleProgramSearchOptions<Kind>& options) -> ModuleProgramProofResults<Kind>;

}  // namespace runir::kr::ps::ext

#include "runir/kr/ps/ext/module_program_executor_impl.hpp"

#endif
