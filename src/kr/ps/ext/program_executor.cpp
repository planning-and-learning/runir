#include "runir/kr/ps/ext/program_executor.hpp"

#include "runir/kr/dl/semantics/detail/repository_instantiations.hpp"
#include "runir/kr/ps/ext/detail/proof_search.hpp"

#include <yggdrasil/formalism/detail/basic_symbol_repository_impl.hpp>

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

template detail::ProgramStep<tyr::GroundTag, InternedExecutionStorage<tyr::GroundTag>>
SuccessorExpander<tyr::GroundTag>::apply_choice<runir::kr::dl::ConceptTag, ProgramStateView<tyr::GroundTag>>(ProgramStateView<tyr::GroundTag>,
                                                                                                             const detail::Choice<runir::kr::dl::ConceptTag>&,
                                                                                                             ProgramSearchStatistics&);

template detail::ProgramStep<tyr::GroundTag, InternedExecutionStorage<tyr::GroundTag>>
SuccessorExpander<tyr::GroundTag>::apply_choice<runir::kr::dl::RoleTag, ProgramStateView<tyr::GroundTag>>(ProgramStateView<tyr::GroundTag>,
                                                                                                          const detail::Choice<runir::kr::dl::RoleTag>&,
                                                                                                          ProgramSearchStatistics&);

template ProgramStateView<tyr::GroundTag>
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::materialize<BuilderProgramStateView<tyr::GroundTag>>(
    const BuilderProgramStateView<tyr::GroundTag>&);

template detail::ProgramStep<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::apply_choice<runir::kr::dl::ConceptTag, BuilderProgramStateView<tyr::GroundTag>>(
    BuilderProgramStateView<tyr::GroundTag>,
    const detail::Choice<runir::kr::dl::ConceptTag>&,
    ProgramSearchStatistics&);

template detail::ProgramStep<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::apply_choice<runir::kr::dl::RoleTag, BuilderProgramStateView<tyr::GroundTag>>(
    BuilderProgramStateView<tyr::GroundTag>,
    const detail::Choice<runir::kr::dl::RoleTag>&,
    ProgramSearchStatistics&);

template ProgramStateView<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag>::materialize<ProgramStateView<tyr::LiftedTag>>(const ProgramStateView<tyr::LiftedTag>&);

template detail::ProgramStep<tyr::LiftedTag, InternedExecutionStorage<tyr::LiftedTag>>
SuccessorExpander<tyr::LiftedTag>::apply_choice<runir::kr::dl::ConceptTag, ProgramStateView<tyr::LiftedTag>>(ProgramStateView<tyr::LiftedTag>,
                                                                                                             const detail::Choice<runir::kr::dl::ConceptTag>&,
                                                                                                             ProgramSearchStatistics&);

template detail::ProgramStep<tyr::LiftedTag, InternedExecutionStorage<tyr::LiftedTag>>
SuccessorExpander<tyr::LiftedTag>::apply_choice<runir::kr::dl::RoleTag, ProgramStateView<tyr::LiftedTag>>(ProgramStateView<tyr::LiftedTag>,
                                                                                                          const detail::Choice<runir::kr::dl::RoleTag>&,
                                                                                                          ProgramSearchStatistics&);

template ProgramStateView<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::materialize<BuilderProgramStateView<tyr::LiftedTag>>(
    const BuilderProgramStateView<tyr::LiftedTag>&);

template detail::ProgramStep<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::apply_choice<runir::kr::dl::ConceptTag, BuilderProgramStateView<tyr::LiftedTag>>(
    BuilderProgramStateView<tyr::LiftedTag>,
    const detail::Choice<runir::kr::dl::ConceptTag>&,
    ProgramSearchStatistics&);

template detail::ProgramStep<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>
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

// Tests intern these records too; their insertion path needs basic_symbol_repository_impl.hpp.
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::CallStack>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::ModuleState<tyr::GroundTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::ModuleState<tyr::LiftedTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::ProgramState<tyr::GroundTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::ProgramState<tyr::LiftedTag>>;
