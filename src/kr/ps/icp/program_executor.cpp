#include "runir/kr/ps/icp/program_executor.hpp"

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include "runir/kr/ps/icp/detail/proof_search.hpp"
#include "runir/kr/ps/icp/repository.hpp"

namespace runir::kr::ps::icp
{

template auto find_solution<tyr::GroundTag>(const TaskContextPtr<tyr::GroundTag>&, ProgramView, const ProgramSearchOptions<tyr::GroundTag>&)
    -> ProgramProofResults<tyr::GroundTag>;

template auto find_solution<tyr::LiftedTag>(const TaskContextPtr<tyr::LiftedTag>&, ProgramView, const ProgramSearchOptions<tyr::LiftedTag>&)
    -> ProgramProofResults<tyr::LiftedTag>;

}

