#ifndef RUNIR_KR_PS_EXT_DL_FORMATTER_HPP_
#define RUNIR_KR_PS_EXT_DL_FORMATTER_HPP_

#include "runir/config.hpp"
#include "runir/kr/ps/dl/formatter.hpp"
#include "runir/kr/ps/ext/dl/incomplete_structural_termination_data.hpp"
#include "runir/kr/ps/ext/dl/structural_termination_data.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <yggdrasil/formatting/dynamic_bitset_formatters.hpp>

#if RUNIR_ENABLE_FMT_FORMATTERS

template<>
struct fmt::formatter<runir::kr::ps::ext::dl::ModulePolicyGraphVertexLabel>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }

    auto format(const runir::kr::ps::ext::dl::ModulePolicyGraphVertexLabel& label, format_context& context) const
    {
        return fmt::format_to(context.out(),
                              "(booleans={}, numericals={}, memory={})",
                              label.boolean_values,
                              label.numerical_values,
                              label.memory_state.get_name());
    }
};

template<>
struct fmt::formatter<runir::kr::ps::ext::dl::ModulePolicyGraphEdgeLabel>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }

    auto format(const runir::kr::ps::ext::dl::ModulePolicyGraphEdgeLabel& label, format_context& context) const
    {
        return fmt::format_to(context.out(), "(rule={}, numerical_changes={})", label.rule.get_symbol(), label.numerical_changes);
    }
};

template<>
struct fmt::formatter<runir::kr::ps::ext::dl::ModuleStructuralTerminationResult>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }

    auto format(const runir::kr::ps::ext::dl::ModuleStructuralTerminationResult& result, format_context& context) const
    {
        if (result.is_terminating())
            return fmt::format_to(context.out(), "ModuleStructuralTerminationResult(terminating)");
        return fmt::format_to(context.out(),
                              "ModuleStructuralTerminationResult(non-terminating, counterexample with {} vertices and {} edges)",
                              result.counterexample->get_num_vertices(),
                              result.counterexample->get_num_edges());
    }
};

template<>
struct fmt::formatter<runir::kr::ps::ext::dl::ModuleProgramStructuralTerminationResult>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }

    auto format(const runir::kr::ps::ext::dl::ModuleProgramStructuralTerminationResult& result, format_context& context) const
    {
        return fmt::format_to(context.out(),
                              "ModuleProgramStructuralTerminationResult({}, {} modules, {} recursive calls)",
                              result.is_terminating() ? "terminating" : "non-terminating",
                              result.module_results.size(),
                              result.recursive_call_rules.size());
    }
};

template<>
struct fmt::formatter<runir::kr::ps::ext::dl::ModuleIncompleteStructuralTerminationResult>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }

    auto format(const runir::kr::ps::ext::dl::ModuleIncompleteStructuralTerminationResult& result, format_context& context) const
    {
        if (result.is_terminating())
            return fmt::format_to(context.out(), "ModuleIncompleteStructuralTerminationResult(terminating)");
        return fmt::format_to(context.out(),
                              "ModuleIncompleteStructuralTerminationResult(unknown, {} surviving rules)",
                              result.surviving_rules.size());
    }
};

template<>
struct fmt::formatter<runir::kr::ps::ext::dl::ModuleProgramIncompleteStructuralTerminationResult>
{
    constexpr auto parse(format_parse_context& context) { return context.begin(); }

    auto format(const runir::kr::ps::ext::dl::ModuleProgramIncompleteStructuralTerminationResult& result, format_context& context) const
    {
        return fmt::format_to(context.out(),
                              "ModuleProgramIncompleteStructuralTerminationResult({}, {} modules, {} recursive calls)",
                              result.is_terminating() ? "terminating" : "unknown",
                              result.module_results.size(),
                              result.recursive_call_rules.size());
    }
};

#endif

#endif
