#ifndef RUNIR_KR_PS_DL_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_DL_STRUCTURAL_TERMINATION_HPP_

#include <cstddef>
#include <cstdint>

namespace runir::kr::ps::dl
{

static constexpr std::size_t default_max_features = 16;
static constexpr bool default_use_incomplete_preprocessing = true;

/// Qualitative change of one numerical(-like) feature along a policy graph
/// edge of the structural termination analyses (see
/// base/dl/structural_termination.hpp and ext/dl/structural_termination.hpp).
enum class NumericalChange : std::uint8_t
{
    UNCONSTRAINED = 0,
    INCREASES = 1,
    DECREASES = 2,
    UNCHANGED = 3,
};

}  // namespace runir::kr::ps::dl

#endif
