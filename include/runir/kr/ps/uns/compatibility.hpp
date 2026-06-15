#ifndef RUNIR_KR_PS_UNS_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_UNS_COMPATIBILITY_HPP_

#include "runir/kr/ps/uns/declarations.hpp"
#include "runir/kr/ps/uns/dl/compatibility.hpp"
#include "runir/kr/ps/uns/rule_view.hpp"
#include "runir/kr/ps/uns/sketch_view.hpp"
#include "runir/kr/ps/compatibility.hpp"

#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::uns
{

template<typename C, typename EvaluationContext>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::uns::Rule>, C> rule, EvaluationContext& context)
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
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::uns::Sketch>, C> sketch, EvaluationContext& context)
{
    for (auto rule : sketch.get_rules())
        if (runir::kr::ps::uns::is_compatible_with(rule, context))
            return true;

    return false;
}

}  // namespace runir::kr::ps::uns

#endif
