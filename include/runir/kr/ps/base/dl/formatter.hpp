#ifndef RUNIR_KR_PS_BASE_DL_FORMATTER_HPP_
#define RUNIR_KR_PS_BASE_DL_FORMATTER_HPP_

#include "runir/formatter.hpp"
#include "runir/kr/ps/base/dl/structural_termination_data.hpp"
#include "runir/kr/ps/dl/incomplete_structural_termination_data.hpp"
#include "runir/kr/ps/formatter.hpp"

#include <fmt/format.h>

// The base family's structural termination results and policy graph labels, as a log line writes them: each formatter's
// format is defined in src/kr/ps/base/dl/formatter.cpp.

/// A policy graph's vertex: (booleans=..., numericals=...).
template<>
struct fmt::formatter<runir::kr::ps::base::dl::PolicyGraphVertexLabel, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::base::dl::PolicyGraphVertexLabel& value, fmt::format_context& context) const;
};

/// A sketch's result: terminating, or non-terminating with its counterexample's size.
template<>
struct fmt::formatter<runir::kr::ps::base::dl::StructuralTerminationResult, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::base::dl::StructuralTerminationResult& value, fmt::format_context& context) const;
};

/// A sketch's incomplete result: terminating, or unknown with its number of surviving rules.
template<>
struct fmt::formatter<runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::BaseFamilyTag>, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::BaseFamilyTag>& value,
                                         fmt::format_context& context) const;
};

#endif
