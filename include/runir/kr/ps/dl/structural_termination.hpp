#ifndef RUNIR_KR_PS_DL_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_DL_STRUCTURAL_TERMINATION_HPP_

#include "runir/config.hpp"

#include <cstdint>

#if RUNIR_ENABLE_FMT_FORMATTERS
#include <fmt/format.h>
#include <string_view>
#endif

namespace runir::kr::ps::dl
{

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

#if RUNIR_ENABLE_FMT_FORMATTERS
namespace fmt
{

template<>
struct formatter<runir::kr::ps::dl::NumericalChange, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(runir::kr::ps::dl::NumericalChange change, FormatContext& ctx) const
    {
        using runir::kr::ps::dl::NumericalChange;
        auto text = std::string_view { "?" };
        switch (change)
        {
            case NumericalChange::UNCONSTRAINED: text = "?"; break;
            case NumericalChange::INCREASES: text = "inc"; break;
            case NumericalChange::DECREASES: text = "dec"; break;
            case NumericalChange::UNCHANGED: text = "unchanged"; break;
        }
        return formatter<std::string_view>::format(text, ctx);
    }
};

}  // namespace fmt
#endif

#endif
