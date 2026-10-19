#include "runir/kr/ps/icp/program_executor.hpp"

#include "runir/kr/dl/detail/repository_instantiations_ext.hpp"
#include "runir/kr/dl/semantics/detail/repository_instantiations.hpp"
#include "runir/kr/ps/icp/detail/proof_search.hpp"
#include "runir/kr/ps/icp/detail/repository_instantiations.hpp"

namespace runir::kr::ps::icp
{

template auto
find_solution<tyr::GroundTag>(TaskContextPtr<tyr::GroundTag>, ProgramView, const ProgramSearchOptions<tyr::GroundTag>&) -> ProgramProofResults<tyr::GroundTag>;

template auto
find_solution<tyr::LiftedTag>(TaskContextPtr<tyr::LiftedTag>, ProgramView, const ProgramSearchOptions<tyr::LiftedTag>&) -> ProgramProofResults<tyr::LiftedTag>;

}
