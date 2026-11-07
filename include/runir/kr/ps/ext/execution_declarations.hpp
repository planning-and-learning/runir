#ifndef RUNIR_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"

#include <memory>
#include <tyr/planning/declarations.hpp>
#include <urd/repr/slots/slots.hpp>
#include <urd/store.hpp>
#include <urd/elements/flat.hpp>

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
class ExecutionRepository;

template<tyr::TaskKind Kind>
class ExecutionRepositoryFactory;

template<tyr::TaskKind Kind>
using ExecutionRepositoryPtr = std::shared_ptr<ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using ExecutionRepositoryFactoryPtr = std::shared_ptr<ExecutionRepositoryFactory<Kind>>;

template<tyr::TaskKind Kind>
using ModuleStateView = urd::index_view<ModuleState<Kind>, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using CallStackView = urd::index_view<CallStack, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using ProgramStateView = urd::index_view<ProgramState<Kind>, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using BorrowedModuleStateView = urd::borrowed_view<ModuleState<Kind>, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using BorrowedCallStackView = urd::borrowed_view<CallStack, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using BorrowedProgramStateView = urd::borrowed_view<ProgramState<Kind>, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using BuilderModuleStateView = urd::borrowed_view<ModuleState<Kind>, ExecutionRepository<Kind>, urd::builder<ModuleState<Kind>>>;

template<tyr::TaskKind Kind>
using BuilderProgramStateView = urd::borrowed_view<ProgramState<Kind>, ExecutionRepository<Kind>, urd::builder<ProgramState<Kind>>>;

}  // namespace runir::kr::ps::ext

#endif
