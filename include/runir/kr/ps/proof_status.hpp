#ifndef RUNIR_KR_PS_PROOF_STATUS_HPP_
#define RUNIR_KR_PS_PROOF_STATUS_HPP_

#include <cstdint>
#include <fmt/format.h>
#include <stdexcept>
#include <string_view>

namespace runir::kr::ps
{

/// How a sketch or program search ended.
enum class ProofStatus : std::uint8_t
{
    SUCCESS,
    FAILURE,
    OUT_OF_TIME,
    OUT_OF_STATES,
};

constexpr std::string_view to_string(ProofStatus status)
{
    switch (status)
    {
        case ProofStatus::SUCCESS:
            return "success";
        case ProofStatus::FAILURE:
            return "failure";
        case ProofStatus::OUT_OF_TIME:
            return "out_of_time";
        case ProofStatus::OUT_OF_STATES:
            return "out_of_states";
    }
    throw std::invalid_argument("invalid ProofStatus");
}

}  // namespace runir::kr::ps

template<>
struct fmt::formatter<runir::kr::ps::ProofStatus> : fmt::formatter<std::string_view>
{
    auto format(runir::kr::ps::ProofStatus status, format_context& ctx) const
    { return fmt::formatter<std::string_view>::format(runir::kr::ps::to_string(status), ctx); }
};

#endif
