#include "runir/kr/ps/ext/program_executor.hpp"

#include "runir/kr/ps/ext/detail/proof_search.hpp"

namespace runir::kr::ps::ext
{

template class SuccessorExpander<tyr::GroundTag>;
template class SuccessorExpander<tyr::LiftedTag>;

template class InternedExecutionStorage<tyr::GroundTag>;

template tyr::planning::LabeledNode<tyr::planning::StateView<tyr::GroundTag>>
InternedExecutionStorage<tyr::GroundTag>::successor<tyr::planning::StateView<tyr::GroundTag>>(const tyr::planning::StateView<tyr::GroundTag>&,
                                                                                              tyr::formalism::planning::ActionBindingView);

template class TransientExecutionStorage<tyr::GroundTag>;

template detail::TransientLabeledNode<tyr::GroundTag>
TransientExecutionStorage<tyr::GroundTag>::successor<tyr::planning::BuilderStateView<tyr::GroundTag>>(const tyr::planning::BuilderStateView<tyr::GroundTag>&,
                                                                                                      tyr::formalism::planning::ActionBindingView);

template class InternedExecutionStorage<tyr::LiftedTag>;

template tyr::planning::LabeledNode<tyr::planning::StateView<tyr::LiftedTag>>
InternedExecutionStorage<tyr::LiftedTag>::successor<tyr::planning::StateView<tyr::LiftedTag>>(const tyr::planning::StateView<tyr::LiftedTag>&,
                                                                                              tyr::formalism::planning::ActionBindingView);

template class TransientExecutionStorage<tyr::LiftedTag>;

template detail::TransientLabeledNode<tyr::LiftedTag>
TransientExecutionStorage<tyr::LiftedTag>::successor<tyr::planning::BuilderStateView<tyr::LiftedTag>>(const tyr::planning::BuilderStateView<tyr::LiftedTag>&,
                                                                                                      tyr::formalism::planning::ActionBindingView);

template ProgramStateView<tyr::GroundTag>
SuccessorExpander<tyr::GroundTag>::materialize<ProgramStateView<tyr::GroundTag>>(const ProgramStateView<tyr::GroundTag>&);

template detail::ProgramStep<tyr::GroundTag>
SuccessorExpander<tyr::GroundTag>::apply_choice<runir::kr::dl::ConceptTag, ProgramStateView<tyr::GroundTag>>(ProgramStateView<tyr::GroundTag>,
                                                                                                             const detail::Choice<runir::kr::dl::ConceptTag>&,
                                                                                                             ProgramSearchStatistics&);

template detail::ProgramStep<tyr::GroundTag>
SuccessorExpander<tyr::GroundTag>::apply_choice<runir::kr::dl::RoleTag, ProgramStateView<tyr::GroundTag>>(ProgramStateView<tyr::GroundTag>,
                                                                                                          const detail::Choice<runir::kr::dl::RoleTag>&,
                                                                                                          ProgramSearchStatistics&);

template ProgramStateView<tyr::GroundTag>
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::materialize<detail::TransientProgramState<tyr::GroundTag>>(
    const detail::TransientProgramState<tyr::GroundTag>&);

template detail::ProgramStep<tyr::GroundTag, detail::TransientProgramState<tyr::GroundTag>>
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::apply_choice<runir::kr::dl::ConceptTag,
                                                                                           detail::TransientProgramState<tyr::GroundTag>>(
    detail::TransientProgramState<tyr::GroundTag>,
    const detail::TransientChoice<runir::kr::dl::ConceptTag>&,
    ProgramSearchStatistics&);

template detail::ProgramStep<tyr::GroundTag, detail::TransientProgramState<tyr::GroundTag>>
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::apply_choice<runir::kr::dl::RoleTag,
                                                                                           detail::TransientProgramState<tyr::GroundTag>>(
    detail::TransientProgramState<tyr::GroundTag>,
    const detail::TransientChoice<runir::kr::dl::RoleTag>&,
    ProgramSearchStatistics&);

template ProgramStateView<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag>::materialize<ProgramStateView<tyr::LiftedTag>>(const ProgramStateView<tyr::LiftedTag>&);

template detail::ProgramStep<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag>::apply_choice<runir::kr::dl::ConceptTag, ProgramStateView<tyr::LiftedTag>>(ProgramStateView<tyr::LiftedTag>,
                                                                                                             const detail::Choice<runir::kr::dl::ConceptTag>&,
                                                                                                             ProgramSearchStatistics&);

template detail::ProgramStep<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag>::apply_choice<runir::kr::dl::RoleTag, ProgramStateView<tyr::LiftedTag>>(ProgramStateView<tyr::LiftedTag>,
                                                                                                          const detail::Choice<runir::kr::dl::RoleTag>&,
                                                                                                          ProgramSearchStatistics&);

template ProgramStateView<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::materialize<detail::TransientProgramState<tyr::LiftedTag>>(
    const detail::TransientProgramState<tyr::LiftedTag>&);

template detail::ProgramStep<tyr::LiftedTag, detail::TransientProgramState<tyr::LiftedTag>>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::apply_choice<runir::kr::dl::ConceptTag,
                                                                                           detail::TransientProgramState<tyr::LiftedTag>>(
    detail::TransientProgramState<tyr::LiftedTag>,
    const detail::TransientChoice<runir::kr::dl::ConceptTag>&,
    ProgramSearchStatistics&);

template detail::ProgramStep<tyr::LiftedTag, detail::TransientProgramState<tyr::LiftedTag>>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::apply_choice<runir::kr::dl::RoleTag,
                                                                                           detail::TransientProgramState<tyr::LiftedTag>>(
    detail::TransientProgramState<tyr::LiftedTag>,
    const detail::TransientChoice<runir::kr::dl::RoleTag>&,
    ProgramSearchStatistics&);

template auto find_solution<tyr::GroundTag>(runir::kr::TaskContextPtr<tyr::GroundTag> task_context,
                                            ProgramView program,
                                            const ProgramSearchOptions<tyr::GroundTag>& options) -> ProgramProofResults<tyr::GroundTag>;

template auto find_solution<tyr::LiftedTag>(runir::kr::TaskContextPtr<tyr::LiftedTag> task_context,
                                            ProgramView program,
                                            const ProgramSearchOptions<tyr::LiftedTag>& options) -> ProgramProofResults<tyr::LiftedTag>;

}  // namespace runir::kr::ps::ext
