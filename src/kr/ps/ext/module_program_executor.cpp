#include "runir/kr/ps/ext/module_program_executor.hpp"

#include "runir/kr/ps/ext/detail/proof_search.hpp"

namespace runir::kr::ps::ext
{

template class SuccessorExpander<tyr::GroundTag>;
template class SuccessorExpander<tyr::LiftedTag>;

template auto find_solution<tyr::GroundTag>(runir::kr::TaskContextPtr<tyr::GroundTag> task_context,
                                                      ModuleProgramView program,
                                                      const ModuleProgramSearchOptions<tyr::GroundTag>& options)
    -> ModuleProgramProofResults<tyr::GroundTag>;

template auto find_solution<tyr::LiftedTag>(runir::kr::TaskContextPtr<tyr::LiftedTag> task_context,
                                                      ModuleProgramView program,
                                                      const ModuleProgramSearchOptions<tyr::LiftedTag>& options)
    -> ModuleProgramProofResults<tyr::LiftedTag>;

}  // namespace runir::kr::ps::ext
