#ifndef RUNIR_KR_DL_SEMANTICS_HEURISTICS_BLIND_HPP_
#define RUNIR_KR_DL_SEMANTICS_HEURISTICS_BLIND_HPP_

#include <cstddef>
#include <span>
#include <yggdrasil/core/config.hpp>

namespace runir::kr::dl::semantics::heuristics
{

/// The zero estimate: the search is breadth-first.
struct BlindHeuristic
{
    ygg::uint_t estimate(std::span<const std::byte>) const noexcept { return 0; }
    template<typename Targets>
    void update(const Targets&) noexcept
    {
    }
};

}  // namespace runir::kr::dl::semantics::heuristics

#endif
