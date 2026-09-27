#ifndef RUNIR_KR_PS_ICP_EXECUTION_DECLARATIONS_HPP_
#define RUNIR_KR_PS_ICP_EXECUTION_DECLARATIONS_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"

#include <memory>
#include <tyr/planning/declarations.hpp>
#include <yggdrasil/core/types.hpp>

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
class ExecutionRepository;

template<tyr::TaskKind Kind>
class ExecutionRepositoryFactory;

template<tyr::TaskKind Kind>
using ExecutionRepositoryPtr = std::shared_ptr<ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using ProgramStateView = ygg::View<ygg::Index<ProgramState<Kind>>, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using HistoriesView = ygg::View<ygg::Index<Histories>, ExecutionRepository<Kind>>;

}

#endif
