#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/ps/ext/detail/proof_search.hpp"
#include "runir/kr/ps/ext/program_executor.hpp"

#include <runir/kr/database/syntax/query_repository.hpp>

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
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::materialize<ProgramStateBuilderView<tyr::LiftedTag>>(
    const ProgramStateBuilderView<tyr::LiftedTag>&);

template ProgramStep<ExtFamilyTag, tyr::LiftedTag, StorageStoredState<TransientExecutionStorage<tyr::LiftedTag>>>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::apply_choice<runir::kr::dl::ConceptTag, ProgramStateBuilderView<tyr::LiftedTag>>(
    ProgramStateBuilderView<tyr::LiftedTag>,
    const detail::Choice<runir::kr::dl::ConceptTag>&,
    ProgramSearchStatistics&);

template ProgramStep<ExtFamilyTag, tyr::LiftedTag, StorageStoredState<TransientExecutionStorage<tyr::LiftedTag>>>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::apply_choice<runir::kr::dl::RoleTag, ProgramStateBuilderView<tyr::LiftedTag>>(
    ProgramStateBuilderView<tyr::LiftedTag>,
    const detail::Choice<runir::kr::dl::RoleTag>&,
    ProgramSearchStatistics&);

template auto find_solution<tyr::LiftedTag>(runir::kr::TaskContextPtr<tyr::LiftedTag> task_context,
                                            ProgramView program,
                                            const ProgramSearchOptions<tyr::LiftedTag>& options) -> ProgramProofResults<tyr::LiftedTag>;

}  // namespace runir::kr::ps::ext
