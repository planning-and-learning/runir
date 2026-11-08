#include "runir/kr/ps/ext/program_executor.hpp"

#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include <yggdrasil/database/syntax/query_repository.hpp>
#include "runir/kr/ps/ext/detail/proof_search.hpp"

namespace runir::kr::ps::ext
{

template class SuccessorExpander<tyr::LiftedTag>;

template class InternedExecutionStorage<tyr::LiftedTag>;
template class TransientExecutionStorage<tyr::LiftedTag>;

template ProgramStateView<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag>::materialize<ProgramStateView<tyr::LiftedTag>>(const ProgramStateView<tyr::LiftedTag>&);

template ProgramStep<ExtFamilyTag, tyr::LiftedTag, StorageStoredState<InternedExecutionStorage<tyr::LiftedTag>>>
SuccessorExpander<tyr::LiftedTag>::apply_choice<runir::kr::dl::ConceptTag, ProgramStateView<tyr::LiftedTag>>(ProgramStateView<tyr::LiftedTag>,
                                                                                                             const detail::Choice<runir::kr::dl::ConceptTag>&,
                                                                                                             ProgramSearchStatistics&);

template ProgramStep<ExtFamilyTag, tyr::LiftedTag, StorageStoredState<InternedExecutionStorage<tyr::LiftedTag>>>
SuccessorExpander<tyr::LiftedTag>::apply_choice<runir::kr::dl::RoleTag, ProgramStateView<tyr::LiftedTag>>(ProgramStateView<tyr::LiftedTag>,
                                                                                                          const detail::Choice<runir::kr::dl::RoleTag>&,
                                                                                                          ProgramSearchStatistics&);

template ProgramStateView<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::materialize<BuilderProgramStateView<tyr::LiftedTag>>(
    const BuilderProgramStateView<tyr::LiftedTag>&);

template ProgramStep<ExtFamilyTag, tyr::LiftedTag, StorageStoredState<TransientExecutionStorage<tyr::LiftedTag>>>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::apply_choice<runir::kr::dl::ConceptTag, BuilderProgramStateView<tyr::LiftedTag>>(
    BuilderProgramStateView<tyr::LiftedTag>,
    const detail::Choice<runir::kr::dl::ConceptTag>&,
    ProgramSearchStatistics&);

template ProgramStep<ExtFamilyTag, tyr::LiftedTag, StorageStoredState<TransientExecutionStorage<tyr::LiftedTag>>>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::apply_choice<runir::kr::dl::RoleTag, BuilderProgramStateView<tyr::LiftedTag>>(
    BuilderProgramStateView<tyr::LiftedTag>,
    const detail::Choice<runir::kr::dl::RoleTag>&,
    ProgramSearchStatistics&);

template auto find_solution<tyr::LiftedTag>(runir::kr::TaskContextPtr<tyr::LiftedTag> task_context,
                                            ProgramView program,
                                            const ProgramSearchOptions<tyr::LiftedTag>& options) -> ProgramProofResults<tyr::LiftedTag>;

}  // namespace runir::kr::ps::ext
