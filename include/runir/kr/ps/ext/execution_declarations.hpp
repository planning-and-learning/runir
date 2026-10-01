#ifndef RUNIR_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"

#include <memory>
#include <type_traits>
#include <tyr/planning/declarations.hpp>
#include <yggdrasil/containers/shared_object_pool.hpp>
#include <yggdrasil/core/types.hpp>

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
using ModuleStateView = ygg::View<ygg::Index<ModuleState<Kind>>, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using CallStackView = ygg::View<ygg::Index<CallStack>, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using ProgramStateView = ygg::View<ygg::Index<ProgramState<Kind>>, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using BorrowedModuleStateView = ygg::View<ygg::Data<ModuleState<Kind>>, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using BorrowedCallStackView = ygg::View<ygg::Data<CallStack>, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using BorrowedProgramStateView = ygg::View<ygg::Data<ProgramState<Kind>>, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using BuilderModuleStateView = ygg::View<ygg::Builder<ModuleState<Kind>>, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using BuilderProgramStateView = ygg::View<ygg::Builder<ProgramState<Kind>>, ExecutionRepository<Kind>>;

/// Values retained by search: repository identities or owners of pooled execution data.
template<typename S, typename Kind>
concept StoredProgramStateConcept = tyr::TaskKind<Kind>
                                    && (std::same_as<std::remove_cvref_t<S>, ProgramStateView<Kind>>
                                        || std::same_as<std::remove_cvref_t<S>, ygg::SharedObjectPoolPtr<ygg::Builder<ProgramState<Kind>>>>);

}  // namespace runir::kr::ps::ext

#endif
