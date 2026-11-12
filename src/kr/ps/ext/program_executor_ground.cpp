#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/ps/ext/detail/proof_search.hpp"
#include "runir/kr/ps/ext/program_executor.hpp"

#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>

namespace runir::kr::ps::ext
{

template class SuccessorExpander<tyr::GroundTag>;

template class InternedExecutionStorage<tyr::GroundTag>;
template class TransientExecutionStorage<tyr::GroundTag>;

template ProgramStateView<tyr::GroundTag>
SuccessorExpander<tyr::GroundTag>::materialize<ProgramStateView<tyr::GroundTag>>(const ProgramStateView<tyr::GroundTag>&);

template ProgramStep<ExtFamilyTag, tyr::GroundTag, StorageStoredState<InternedExecutionStorage<tyr::GroundTag>>>
SuccessorExpander<tyr::GroundTag>::apply_choice<runir::kr::dl::ConceptTag, ProgramStateView<tyr::GroundTag>>(ProgramStateView<tyr::GroundTag>,
                                                                                                             const detail::Choice<runir::kr::dl::ConceptTag>&,
                                                                                                             ProgramSearchStatistics&);

template ProgramStep<ExtFamilyTag, tyr::GroundTag, StorageStoredState<InternedExecutionStorage<tyr::GroundTag>>>
SuccessorExpander<tyr::GroundTag>::apply_choice<runir::kr::dl::RoleTag, ProgramStateView<tyr::GroundTag>>(ProgramStateView<tyr::GroundTag>,
                                                                                                          const detail::Choice<runir::kr::dl::RoleTag>&,
                                                                                                          ProgramSearchStatistics&);

template ProgramStateView<tyr::GroundTag>
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::materialize<BuilderProgramStateView<tyr::GroundTag>>(
    const BuilderProgramStateView<tyr::GroundTag>&);

template ProgramStep<ExtFamilyTag, tyr::GroundTag, StorageStoredState<TransientExecutionStorage<tyr::GroundTag>>>
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::apply_choice<runir::kr::dl::ConceptTag, BuilderProgramStateView<tyr::GroundTag>>(
    BuilderProgramStateView<tyr::GroundTag>,
    const detail::Choice<runir::kr::dl::ConceptTag>&,
    ProgramSearchStatistics&);

template ProgramStep<ExtFamilyTag, tyr::GroundTag, StorageStoredState<TransientExecutionStorage<tyr::GroundTag>>>
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::apply_choice<runir::kr::dl::RoleTag, BuilderProgramStateView<tyr::GroundTag>>(
    BuilderProgramStateView<tyr::GroundTag>,
    const detail::Choice<runir::kr::dl::RoleTag>&,
    ProgramSearchStatistics&);

template auto find_solution<tyr::GroundTag>(runir::kr::TaskContextPtr<tyr::GroundTag> task_context,
                                            ProgramView program,
                                            const ProgramSearchOptions<tyr::GroundTag>& options) -> ProgramProofResults<tyr::GroundTag>;

}  // namespace runir::kr::ps::ext
