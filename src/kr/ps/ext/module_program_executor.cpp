#include "runir/kr/ps/ext/module_program_executor_impl.hpp"

#ifndef RUNIR_HEADER_INSTANTIATION
namespace runir::kr::ps::ext
{

template auto prove_solution<tyr::planning::GroundTag>(datasets::TaskSearchContextPtr<tyr::planning::GroundTag> context,
                                                       ModuleProgramView program,
                                                       const ModuleProgramSearchOptions<tyr::planning::GroundTag>& options)
    -> ModuleProgramProofResults<tyr::planning::GroundTag>;

template auto prove_solution<tyr::planning::LiftedTag>(datasets::TaskSearchContextPtr<tyr::planning::LiftedTag> context,
                                                       ModuleProgramView program,
                                                       const ModuleProgramSearchOptions<tyr::planning::LiftedTag>& options)
    -> ModuleProgramProofResults<tyr::planning::LiftedTag>;

template auto find_solution<tyr::planning::GroundTag>(datasets::TaskSearchContextPtr<tyr::planning::GroundTag> context,
                                                      ModuleProgramView program,
                                                      const ModuleProgramSearchOptions<tyr::planning::GroundTag>& options)
    -> ModuleProgramProofResults<tyr::planning::GroundTag>;

template auto find_solution<tyr::planning::LiftedTag>(datasets::TaskSearchContextPtr<tyr::planning::LiftedTag> context,
                                                      ModuleProgramView program,
                                                      const ModuleProgramSearchOptions<tyr::planning::LiftedTag>& options)
    -> ModuleProgramProofResults<tyr::planning::LiftedTag>;

}  // namespace runir::kr::ps::ext
#endif
