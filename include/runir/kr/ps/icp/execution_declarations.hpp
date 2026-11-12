#ifndef RUNIR_KR_PS_ICP_EXECUTION_DECLARATIONS_HPP_
#define RUNIR_KR_PS_ICP_EXECUTION_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <boost/mp11.hpp>
#include <tyr/planning/declarations.hpp>
#include <urd/elements/flat.hpp>

namespace runir::kr::ps::icp
{

struct Histories
{
};

template<tyr::TaskKind Kind>
struct ProgramState
{
};

template<tyr::TaskKind Kind>
using ExecutionRepositoryTypes = boost::mp11::mp_list<Histories, ProgramState<Kind>>;

template<tyr::TaskKind Kind>
using ProgramStateView = urd::index_view<ProgramState<Kind>, ExecutionRepository<IcpFamilyTag, Kind>>;

template<tyr::TaskKind Kind>
using HistoriesView = urd::index_view<Histories, ExecutionRepository<IcpFamilyTag, Kind>>;

}

#endif
