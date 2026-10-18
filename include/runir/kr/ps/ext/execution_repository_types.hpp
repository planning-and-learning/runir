#ifndef RUNIR_KR_PS_EXT_EXECUTION_REPOSITORY_TYPES_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_REPOSITORY_TYPES_HPP_

#include "runir/kr/ps/ext/execution_builder.hpp"
#include "runir/kr/ps/ext/execution_data.hpp"

#include <tyr/declarations.hpp>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/formalism/builder.hpp>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind>
using ExecutionRepositoryTypes = ygg::TypeList<ModuleState<Kind>, CallStack, ProgramState<Kind>>;

template<tyr::TaskKind Kind>
using ExecutionBuilder = ygg::ApplyTypeListT<ygg::formalism::BuilderStorage, ExecutionRepositoryTypes<Kind>>;

}

#endif
