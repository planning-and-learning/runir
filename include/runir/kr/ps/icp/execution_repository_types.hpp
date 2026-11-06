#ifndef RUNIR_KR_PS_ICP_EXECUTION_REPOSITORY_TYPES_HPP_
#define RUNIR_KR_PS_ICP_EXECUTION_REPOSITORY_TYPES_HPP_

#include "runir/kr/ps/icp/execution_data.hpp"

#include <tyr/declarations.hpp>
#include <boost/mp11.hpp>
#include <yggdrasil/formalism/builder.hpp>

namespace runir::kr::ps::icp
{

template<tyr::TaskKind Kind>
using ExecutionRepositoryTypes = boost::mp11::mp_list<Histories, ProgramState<Kind>>;

template<tyr::TaskKind Kind>
using ExecutionBuilder = boost::mp11::mp_apply<ygg::formalism::BuilderStorage, ExecutionRepositoryTypes<Kind>>;

}

#endif
