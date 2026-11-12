#ifndef RUNIR_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <boost/mp11.hpp>
#include <tyr/planning/declarations.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/slots/slots.hpp>
#include <urd/store.hpp>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind>
struct ModuleState
{
};

struct CallStack
{
};

template<tyr::TaskKind Kind>
struct ProgramState
{
};

template<tyr::TaskKind Kind>
using ExecutionRepositoryTypes = boost::mp11::mp_list<ModuleState<Kind>, CallStack, ProgramState<Kind>>;

template<tyr::TaskKind Kind>
using ModuleStateView = urd::index_view<ModuleState<Kind>, ExecutionRepository<ExtFamilyTag, Kind>>;

template<tyr::TaskKind Kind>
using CallStackView = urd::index_view<CallStack, ExecutionRepository<ExtFamilyTag, Kind>>;

template<tyr::TaskKind Kind>
using ProgramStateView = urd::index_view<ProgramState<Kind>, ExecutionRepository<ExtFamilyTag, Kind>>;

template<tyr::TaskKind Kind>
using BorrowedProgramStateView = urd::borrowed_view<ProgramState<Kind>, ExecutionRepository<ExtFamilyTag, Kind>>;

template<tyr::TaskKind Kind>
using BuilderModuleStateView = urd::borrowed_view<ModuleState<Kind>, ExecutionRepository<ExtFamilyTag, Kind>, urd::builder<ModuleState<Kind>>>;

template<tyr::TaskKind Kind>
using BuilderProgramStateView = urd::borrowed_view<ProgramState<Kind>, ExecutionRepository<ExtFamilyTag, Kind>, urd::builder<ProgramState<Kind>>>;

}  // namespace runir::kr::ps::ext

#endif
