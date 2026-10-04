#ifndef RUNIR_KR_PS_BASE_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_BASE_COMPATIBILITY_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/ps/compatibility.hpp"
#include "runir/kr/ps/dl/compatibility.hpp"

#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::base
{

template<typename C, typename Context>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>, C> rule, Context& context)
{
    return all_compatible(rule.get_conditions(), context) && all_compatible(rule.get_effects(), context);
}

template<typename C, typename Context>
bool is_compatible_with(ygg::View<ygg::Index<runir::kr::ps::base::Sketch>, C> sketch, Context& context)
{
    for (auto rule : sketch.get_rules())
        if (runir::kr::ps::base::is_compatible_with(rule, context))
            return true;

    return false;
}

}  // namespace runir::kr::ps::base

#endif
