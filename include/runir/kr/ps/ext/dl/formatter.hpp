#ifndef RUNIR_KR_PS_EXT_DL_FORMATTER_HPP_
#define RUNIR_KR_PS_EXT_DL_FORMATTER_HPP_

#include "runir/formatter.hpp"
#include "runir/kr/ps/ext/dl/incomplete_structural_termination_data.hpp"
#include "runir/kr/ps/ext/dl/structural_termination_data.hpp"

#include <fmt/format.h>

// The ext family's structural termination results and policy graph labels, as a log line writes them: each formatter's
// format is defined in src/kr/ps/ext/dl/formatter.cpp.

/// A module policy graph's vertex: (booleans=..., numericals=..., memory=name).
template<>
struct fmt::formatter<runir::kr::ps::ext::dl::ModulePolicyGraphVertexLabel> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::ext::dl::ModulePolicyGraphVertexLabel& value, fmt::format_context& context) const;
};

/// A module's result: terminating, or non-terminating with its counterexample's size.
template<>
struct fmt::formatter<runir::kr::ps::ext::dl::ModuleStructuralTerminationResult> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::ext::dl::ModuleStructuralTerminationResult& value, fmt::format_context& context) const;
};

/// A program's result: its verdict, its number of modules and of recursive calls.
template<>
struct fmt::formatter<runir::kr::ps::ext::dl::ProgramStructuralTerminationResult> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::ext::dl::ProgramStructuralTerminationResult& value, fmt::format_context& context) const;
};

/// A module's incomplete result: terminating, or unknown with its number of surviving rules.
template<>
struct fmt::formatter<runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::ExtFamilyTag>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::ExtFamilyTag>& value,
                                         fmt::format_context& context) const;
};

/// A program's incomplete result: its verdict, its number of modules and of recursive calls.
template<>
struct fmt::formatter<runir::kr::ps::ext::dl::ProgramIncompleteStructuralTerminationResult> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::ext::dl::ProgramIncompleteStructuralTerminationResult& value, fmt::format_context& context) const;
};

#endif
