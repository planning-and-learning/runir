#ifndef RUNIR_KR_PS_EXT_MEMORY_STATE_HPP_
#define RUNIR_KR_PS_EXT_MEMORY_STATE_HPP_

#include "runir/kr/ps/ext/memory_state_data.hpp"
#include "runir/kr/ps/ext/memory_state_index.hpp"
#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <tuple>
#include <variant>

namespace runir::kr::ps::ext
{

struct InternalMemoryState
{
    MemoryStateView value;

    explicit InternalMemoryState(MemoryStateView value_) noexcept : value(value_) {}

    auto identifying_members() const noexcept { return std::tie(value); }
};

struct ExternalMemoryState
{
    MemoryStateView value;

    explicit ExternalMemoryState(MemoryStateView value_) noexcept : value(value_) {}

    auto identifying_members() const noexcept { return std::tie(value); }
};

using MemoryStateVariant = std::variant<InternalMemoryState, ExternalMemoryState>;

}  // namespace runir::kr::ps::ext

#endif
