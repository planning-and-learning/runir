#ifndef RUNIR_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_

#include <cstdint>
#include <memory>
#include <tyr/planning/declarations.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::ext
{

struct RegisterValues
{
};

struct CallArguments
{
};

struct CallStack
{
};

enum class ExecutionPhase : std::uint8_t
{
    INTERNAL,
    EXTERNAL,
};

template<tyr::planning::TaskKind Kind>
struct ExecutionState
{
};

template<tyr::planning::TaskKind Kind>
class ExecutionRepository;

template<tyr::planning::TaskKind Kind>
class ExecutionRepositoryFactory;

template<tyr::planning::TaskKind Kind>
using ExecutionRepositoryPtr = std::shared_ptr<ExecutionRepository<Kind>>;

template<tyr::planning::TaskKind Kind>
using ExecutionRepositoryFactoryPtr = std::shared_ptr<ExecutionRepositoryFactory<Kind>>;

template<tyr::planning::TaskKind Kind>
using RegisterValuesView = ygg::View<ygg::Index<RegisterValues>, ExecutionRepository<Kind>>;

template<tyr::planning::TaskKind Kind>
using CallArgumentsView = ygg::View<ygg::Index<CallArguments>, ExecutionRepository<Kind>>;

template<tyr::planning::TaskKind Kind>
using CallStackView = ygg::View<ygg::Index<CallStack>, ExecutionRepository<Kind>>;

template<tyr::planning::TaskKind Kind>
using ExecutionStateView = ygg::View<ygg::Index<ExecutionState<Kind>>, ExecutionRepository<Kind>>;

}  // namespace runir::kr::ps::ext

#endif
