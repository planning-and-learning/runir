#include "runir/kr/ps/ext/program_executor.hpp"

#include "runir/kr/ps/ext/detail/proof_search.hpp"

namespace runir::kr::ps::ext
{

template class SuccessorExpander<tyr::GroundTag>;
template class SuccessorExpander<tyr::LiftedTag>;

template auto find_solution<tyr::GroundTag>(runir::kr::TaskContextPtr<tyr::GroundTag> task_context,
                                                      ProgramView program,
                                                      const ProgramSearchOptions<tyr::GroundTag>& options)
    -> ProgramProofResults<tyr::GroundTag>;

template auto find_solution<tyr::LiftedTag>(runir::kr::TaskContextPtr<tyr::LiftedTag> task_context,
                                                      ProgramView program,
                                                      const ProgramSearchOptions<tyr::LiftedTag>& options)
    -> ProgramProofResults<tyr::LiftedTag>;

}  // namespace runir::kr::ps::ext
