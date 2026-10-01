#include "runir/kr/ps/ext/program_executor.hpp"

#include "runir/kr/ps/ext/detail/proof_search.hpp"

namespace runir::kr::ps::ext
{

template class SuccessorExpander<tyr::GroundTag>;
template class SuccessorExpander<tyr::LiftedTag>;

template class InternedExecutionStorage<tyr::GroundTag>;
template class TransientExecutionStorage<tyr::GroundTag>;
template class InternedExecutionStorage<tyr::LiftedTag>;
template class TransientExecutionStorage<tyr::LiftedTag>;

template ProgramStateView<tyr::GroundTag>
SuccessorExpander<tyr::GroundTag>::materialize<ProgramStateView<tyr::GroundTag>>(const ProgramStateView<tyr::GroundTag>&);

template auto
SuccessorExpander<tyr::GroundTag>::apply_choice<runir::kr::dl::ConceptTag, ProgramStateView<tyr::GroundTag>>(ProgramStateView<tyr::GroundTag>,
                                                                                                             const detail::Choice<runir::kr::dl::ConceptTag>&,
                                                                                                             ProgramSearchStatistics&);

template auto
SuccessorExpander<tyr::GroundTag>::apply_choice<runir::kr::dl::RoleTag, ProgramStateView<tyr::GroundTag>>(ProgramStateView<tyr::GroundTag>,
                                                                                                          const detail::Choice<runir::kr::dl::RoleTag>&,
                                                                                                          ProgramSearchStatistics&);

template ProgramStateView<tyr::GroundTag>
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::materialize<BuilderProgramStateView<tyr::GroundTag>>(
    const BuilderProgramStateView<tyr::GroundTag>&);

template auto
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::apply_choice<runir::kr::dl::ConceptTag, BuilderProgramStateView<tyr::GroundTag>>(
    BuilderProgramStateView<tyr::GroundTag>,
    const detail::Choice<runir::kr::dl::ConceptTag>&,
    ProgramSearchStatistics&);

template auto
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::apply_choice<runir::kr::dl::RoleTag, BuilderProgramStateView<tyr::GroundTag>>(
    BuilderProgramStateView<tyr::GroundTag>,
    const detail::Choice<runir::kr::dl::RoleTag>&,
    ProgramSearchStatistics&);

template ProgramStateView<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag>::materialize<ProgramStateView<tyr::LiftedTag>>(const ProgramStateView<tyr::LiftedTag>&);

template auto
SuccessorExpander<tyr::LiftedTag>::apply_choice<runir::kr::dl::ConceptTag, ProgramStateView<tyr::LiftedTag>>(ProgramStateView<tyr::LiftedTag>,
                                                                                                             const detail::Choice<runir::kr::dl::ConceptTag>&,
                                                                                                             ProgramSearchStatistics&);

template auto
SuccessorExpander<tyr::LiftedTag>::apply_choice<runir::kr::dl::RoleTag, ProgramStateView<tyr::LiftedTag>>(ProgramStateView<tyr::LiftedTag>,
                                                                                                          const detail::Choice<runir::kr::dl::RoleTag>&,
                                                                                                          ProgramSearchStatistics&);

template ProgramStateView<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::materialize<BuilderProgramStateView<tyr::LiftedTag>>(
    const BuilderProgramStateView<tyr::LiftedTag>&);

template auto
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::apply_choice<runir::kr::dl::ConceptTag, BuilderProgramStateView<tyr::LiftedTag>>(
    BuilderProgramStateView<tyr::LiftedTag>,
    const detail::Choice<runir::kr::dl::ConceptTag>&,
    ProgramSearchStatistics&);

template auto
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::apply_choice<runir::kr::dl::RoleTag, BuilderProgramStateView<tyr::LiftedTag>>(
    BuilderProgramStateView<tyr::LiftedTag>,
    const detail::Choice<runir::kr::dl::RoleTag>&,
    ProgramSearchStatistics&);

template auto find_solution<tyr::GroundTag>(runir::kr::TaskContextPtr<tyr::GroundTag> task_context,
                                            ProgramView program,
                                            const ProgramSearchOptions<tyr::GroundTag>& options) -> ProgramProofResults<tyr::GroundTag>;

template auto find_solution<tyr::LiftedTag>(runir::kr::TaskContextPtr<tyr::LiftedTag> task_context,
                                            ProgramView program,
                                            const ProgramSearchOptions<tyr::LiftedTag>& options) -> ProgramProofResults<tyr::LiftedTag>;

}  // namespace runir::kr::ps::ext
