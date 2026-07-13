#ifndef RUNIR_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_

#include <cstdint>
#include <memory>
#include <tyr/planning/declarations.hpp>

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
using ExecutionRepositoryPtr = std::shared_ptr<ExecutionRepository<Kind>>;

}  // namespace runir::kr::ps::ext

#endif
