#ifndef RUNIR_KR_PS_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_COMPATIBILITY_HPP_

#include "runir/kr/dl/semantics/evaluation_context.hpp"
#include "runir/kr/ps/dl/compatibility.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <concepts>
#include <ranges>
#include <variant>

namespace runir::kr::ps
{

/// A context that conditions and effects of `Family` can be checked against: a state or a transition evaluation context.
template<typename C, typename Family, typename Kind>
concept CompatibilityContext =
    runir::kr::FamilyTag<Family> && tyr::TaskKind<Kind>
    && (runir::kr::dl::semantics::EvaluationContextConcept<C, Kind> || runir::kr::ps::dl::TransitionEvaluationContextConcept<C, Family, Kind>);

template<tyr::TaskKind Kind, runir::kr::FamilyTag Family, std::ranges::input_range Range, CompatibilityContext<Family, Kind> Context>
bool all_compatible(const Range& values, Context& context)
{
    for (const auto value : values)
        if (!std::visit([&](auto concrete) { return runir::kr::ps::is_compatible_with<Kind>(concrete, context); }, value))
            return false;
    return true;
}

}  // namespace runir::kr::ps

#endif
