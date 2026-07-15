#include "runir/kr/ps/ext/module_program_executor.hpp"

namespace runir::kr::ps::ext
{

template class SuccessorExpander<tyr::planning::GroundTag>;
template class SuccessorExpander<tyr::planning::LiftedTag>;

template auto find_solution<tyr::planning::GroundTag>(runir::kr::TaskContextPtr<tyr::planning::GroundTag> task_context,
                                                      ModuleProgramView program,
                                                      const ModuleProgramSearchOptions<tyr::planning::GroundTag>& options)
    -> ModuleProgramProofResults<tyr::planning::GroundTag>;

template auto find_solution<tyr::planning::LiftedTag>(runir::kr::TaskContextPtr<tyr::planning::LiftedTag> task_context,
                                                      ModuleProgramView program,
                                                      const ModuleProgramSearchOptions<tyr::planning::LiftedTag>& options)
    -> ModuleProgramProofResults<tyr::planning::LiftedTag>;

}  // namespace runir::kr::ps::ext
