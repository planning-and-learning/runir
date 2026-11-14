#ifndef RUNIR_KR_PS_ICP_EXECUTION_REPOSITORY_TYPES_HPP_
#define RUNIR_KR_PS_ICP_EXECUTION_REPOSITORY_TYPES_HPP_

#include "runir/kr/ps/icp/execution_data.hpp"

#include <boost/mp11.hpp>
#include <tyr/declarations.hpp>
#include <yggdrasil/formalism/builder.hpp>

namespace runir::kr::ps::icp
{

template<tyr::TaskKind Kind>
using ExecutionBuilder = boost::mp11::mp_apply<ygg::formalism::BuilderStorage, ExecutionRepositoryTypes<Kind>>;

/// The execution repositories of the icp family, by task kind.
using GroundExecutionRepository = runir::kr::ps::ExecutionRepository<runir::kr::IcpFamilyTag, tyr::GroundTag>;
using LiftedExecutionRepository = runir::kr::ps::ExecutionRepository<runir::kr::IcpFamilyTag, tyr::LiftedTag>;
using GroundExecutionBuilder = ExecutionBuilder<tyr::GroundTag>;
using LiftedExecutionBuilder = ExecutionBuilder<tyr::LiftedTag>;

}

#endif
