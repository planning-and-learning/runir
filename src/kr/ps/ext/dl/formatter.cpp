#include "runir/kr/ps/ext/dl/formatter.hpp"

#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/memory_state_view.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <urd/config.hpp>
#include <urd/format.hpp>
#include <yggdrasil/formatting/cista_formatters.hpp>

namespace ped = runir::kr::ps::ext::dl;

fmt::format_context::iterator fmt::formatter<ped::ModulePolicyGraphVertexLabel>::format(const ped::ModulePolicyGraphVertexLabel& value,
                                                                                        fmt::format_context& context) const
{ return fmt::format_to(context.out(), "(booleans={}, numericals={}, memory={})", value.boolean_values, value.numerical_values, value.memory_state.name()); }

fmt::format_context::iterator fmt::formatter<ped::ModuleStructuralTerminationResult>::format(const ped::ModuleStructuralTerminationResult& value,
                                                                                             fmt::format_context& context) const
{
    if (value.is_terminating())
        return fmt::format_to(context.out(), "ModuleStructuralTerminationResult(terminating)");
    // A non-terminating module has a SIEVE counterexample.
    URD_ASSUME(value.sieve_result.has_value());
    return fmt::format_to(context.out(),
                          "ModuleStructuralTerminationResult(non-terminating, counterexample with {} vertices and {} edges)",
                          value.sieve_result->counterexample->num_vertices(),
                          value.sieve_result->counterexample->num_edges());
}

fmt::format_context::iterator fmt::formatter<ped::ProgramStructuralTerminationResult>::format(const ped::ProgramStructuralTerminationResult& value,
                                                                                              fmt::format_context& context) const
{
    return fmt::format_to(context.out(),
                          "ProgramStructuralTerminationResult({}, {} modules, {} recursive calls)",
                          value.is_terminating() ? "terminating" : "non-terminating",
                          value.module_results.size(),
                          value.recursive_call_rules.size());
}

fmt::format_context::iterator fmt::formatter<runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::ExtFamilyTag>>::format(
    const runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::ExtFamilyTag>& value,
    fmt::format_context& context) const
{
    if (value.is_terminating())
        return fmt::format_to(context.out(), "ModuleIncompleteStructuralTerminationResult(terminating)");
    return fmt::format_to(context.out(), "ModuleIncompleteStructuralTerminationResult(unknown, {} surviving rules)", value.surviving_rules.size());
}

fmt::format_context::iterator
fmt::formatter<ped::ProgramIncompleteStructuralTerminationResult>::format(const ped::ProgramIncompleteStructuralTerminationResult& value,
                                                                          fmt::format_context& context) const
{
    return fmt::format_to(context.out(),
                          "ProgramIncompleteStructuralTerminationResult({}, {} modules, {} recursive calls)",
                          value.is_terminating() ? "terminating" : "unknown",
                          value.module_results.size(),
                          value.recursive_call_rules.size());
}
