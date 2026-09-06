#ifndef RUNIR_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string_view>
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

constexpr std::string_view to_string(ExecutionPhase phase)
{
    switch (phase)
    {
        case ExecutionPhase::INTERNAL:
            return "INTERNAL";
        case ExecutionPhase::EXTERNAL:
            return "EXTERNAL";
    }
    throw std::invalid_argument("invalid ExecutionPhase");
}

template<tyr::TaskKind Kind>
struct ExecutionState
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
using RegisterValuesView = ygg::View<ygg::Index<RegisterValues>, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using CallArgumentsView = ygg::View<ygg::Index<CallArguments>, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using CallStackView = ygg::View<ygg::Index<CallStack>, ExecutionRepository<Kind>>;

template<tyr::TaskKind Kind>
using ExecutionStateView = ygg::View<ygg::Index<ExecutionState<Kind>>, ExecutionRepository<Kind>>;

}  // namespace runir::kr::ps::ext

#endif
