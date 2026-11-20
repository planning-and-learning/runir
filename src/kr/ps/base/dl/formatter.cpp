#include "runir/kr/ps/base/dl/formatter.hpp"

#include "runir/kr/ps/base/repository.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <urd/format.hpp>

namespace pbd = runir::kr::ps::base::dl;

fmt::format_context::iterator fmt::formatter<pbd::PolicyGraphVertexLabel, char>::format(const pbd::PolicyGraphVertexLabel& value,
                                                                                        fmt::format_context& context) const
{ return fmt::format_to(context.out(), "(booleans={}, numericals={})", value.boolean_values, value.numerical_values); }

fmt::format_context::iterator fmt::formatter<pbd::StructuralTerminationResult, char>::format(const pbd::StructuralTerminationResult& value,
                                                                                             fmt::format_context& context) const
{
    if (value.is_terminating())
        return fmt::format_to(context.out(), "StructuralTerminationResult(terminating)");
    // The counterexample is SIEVE's, where it ran.
    const auto& sieve = value.sieve_result;
    if (!sieve.has_value())
        return fmt::format_to(context.out(), "StructuralTerminationResult(non-terminating)");
    return fmt::format_to(context.out(),
                          "StructuralTerminationResult(non-terminating, counterexample with {} vertices and {} edges)",
                          sieve->counterexample->num_vertices(),
                          sieve->counterexample->num_edges());
}

fmt::format_context::iterator fmt::formatter<runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::BaseFamilyTag>, char>::format(
    const runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::BaseFamilyTag>& value,
    fmt::format_context& context) const
{
    if (value.is_terminating())
        return fmt::format_to(context.out(), "IncompleteStructuralTerminationResult(terminating)");
    return fmt::format_to(context.out(), "IncompleteStructuralTerminationResult(unknown, {} surviving rules)", value.surviving_rules.size());
}
