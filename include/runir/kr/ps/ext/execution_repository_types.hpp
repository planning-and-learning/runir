#ifndef RUNIR_KR_PS_EXT_EXECUTION_REPOSITORY_TYPES_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_REPOSITORY_TYPES_HPP_

#include "runir/kr/ps/ext/execution_builder.hpp"
#include "runir/kr/ps/ext/execution_data.hpp"

#include <boost/mp11.hpp>
#include <tyr/declarations.hpp>
#include <yggdrasil/formalism/builder.hpp>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind>
using ExecutionBuilder = boost::mp11::mp_apply<ygg::formalism::BuilderStorage, ExecutionRepositoryTypes<Kind>>;

/// The execution repositories of the ext family, by task kind.
using GroundExecutionRepository = runir::kr::ps::ExecutionRepository<runir::kr::ExtFamilyTag, tyr::GroundTag>;
using LiftedExecutionRepository = runir::kr::ps::ExecutionRepository<runir::kr::ExtFamilyTag, tyr::LiftedTag>;
using GroundExecutionBuilder = ExecutionBuilder<tyr::GroundTag>;
using LiftedExecutionBuilder = ExecutionBuilder<tyr::LiftedTag>;

}

#endif
