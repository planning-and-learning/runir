#ifndef RUNIR_KR_PS_EXT_EXECUTION_REPOSITORY_TYPES_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_REPOSITORY_TYPES_HPP_

#include "runir/kr/ps/ext/execution_builder.hpp"
#include "runir/kr/ps/ext/execution_data.hpp"

#include <tyr/declarations.hpp>
#include <boost/mp11.hpp>
#include <yggdrasil/formalism/builder.hpp>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind>
using ExecutionRepositoryTypes = boost::mp11::mp_list<ModuleState<Kind>, CallStack, ProgramState<Kind>>;

template<tyr::TaskKind Kind>
using ExecutionBuilder = boost::mp11::mp_apply<ygg::formalism::BuilderStorage, ExecutionRepositoryTypes<Kind>>;

}

#endif
