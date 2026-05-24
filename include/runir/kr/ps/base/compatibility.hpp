#ifndef RUNIR_KR_PS_BASE_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_BASE_COMPATIBILITY_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/dl/compatibility.hpp"
#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/ps/compatibility.hpp"

#include <tyr/common/types.hpp>

namespace runir::kr::ps::base
{

template<typename C, typename EvaluationContext>
bool is_compatible_with(tyr::View<tyr::Index<runir::kr::ps::base::Rule>, C> rule, EvaluationContext& context)
{
    for (auto condition : rule.get_conditions())
        if (!runir::kr::ps::is_compatible_with(condition, context))
            return false;

    for (auto effect : rule.get_effects())
        if (!runir::kr::ps::is_compatible_with(effect, context))
            return false;

    return true;
}

template<typename C, typename EvaluationContext>
bool is_compatible_with(tyr::View<tyr::Index<runir::kr::ps::base::Sketch>, C> sketch, EvaluationContext& context)
{
    for (auto rule : sketch.get_rules())
        if (runir::kr::ps::base::is_compatible_with(rule, context))
            return true;

    return false;
}

}  // namespace runir::kr::ps::base

#endif
