#ifndef RUNIR_KR_PS_DL_FORMATTER_HPP_
#define RUNIR_KR_PS_DL_FORMATTER_HPP_

#include "runir/config.hpp"
#include "runir/kr/ps/dl/structural_termination.hpp"

#include <fmt/format.h>
#include <string_view>

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
            case NumericalChange::UNCONSTRAINED:
                text = "?";
                break;
            case NumericalChange::INCREASES:
                text = "inc";
                break;
            case NumericalChange::DECREASES:
                text = "dec";
                break;
            case NumericalChange::UNCHANGED:
                text = "unchanged";
                break;
        }
        return formatter<std::string_view>::format(text, ctx);
    }
};

}  // namespace fmt

#endif
