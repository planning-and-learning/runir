#ifndef RUNIR_KR_PS_DL_FORMATTER_HPP_
#define RUNIR_KR_PS_DL_FORMATTER_HPP_

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
        return formatter<std::string_view>::format(runir::kr::ps::dl::to_string(change), ctx);
    }
};

}  // namespace fmt

#endif
