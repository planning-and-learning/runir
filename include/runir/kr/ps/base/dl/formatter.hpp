#ifndef RUNIR_KR_PS_BASE_DL_FORMATTER_HPP_
#define RUNIR_KR_PS_BASE_DL_FORMATTER_HPP_

#include "runir/kr/ps/base/dl/incomplete_structural_termination_data.hpp"
#include "runir/kr/ps/base/dl/structural_termination_data.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/formatter.hpp"

#include <fmt/format.h>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <yggdrasil/formatting/dynamic_bitset_formatters.hpp>
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::ps::base::dl
{

inline std::string symbol_section(std::string_view value) { return fmt::format("(:symbol {})", value); }

}  // namespace runir::kr::ps::base::dl

template<>
struct fmt::formatter<runir::kr::ps::base::dl::PolicyGraphVertexLabel, char> : fmt::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::kr::ps::base::dl::PolicyGraphVertexLabel& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("(booleans={}, numericals={})", label.boolean_values, label.numerical_values);
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct fmt::formatter<runir::kr::ps::base::dl::StructuralTerminationResult, char> : fmt::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::kr::ps::base::dl::StructuralTerminationResult& result, FormatContext& ctx) const
    {
        const auto text = result.is_terminating() ? std::string { "StructuralTerminationResult(terminating)" } :
                                                    fmt::format("StructuralTerminationResult(non-terminating, counterexample with {} vertices and {} edges)",
                                                                result.sieve_result->counterexample->get_num_vertices(),
                                                                result.sieve_result->counterexample->get_num_edges());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct fmt::formatter<runir::kr::ps::base::dl::IncompleteStructuralTerminationResult, char> : fmt::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::kr::ps::base::dl::IncompleteStructuralTerminationResult& result, FormatContext& ctx) const
    {
        const auto text = result.is_terminating() ?
                              std::string { "IncompleteStructuralTerminationResult(terminating)" } :
                              fmt::format("IncompleteStructuralTerminationResult(unknown, {} surviving rules)", result.surviving_rules.size());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

#endif
