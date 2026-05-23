#include "pyrunir/kr/ps/base/module.hpp"

#include <runir/kr/ps/base/repository.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::ps::base
{

void bind_indices(nb::module_& m)
{
    tyr::bind_index<tyr::Index<ConditionVariant<runir::kr::BaseFamilyTag>>>(m, "ConditionVariantIndex");
    tyr::bind_index<tyr::Index<EffectVariant<runir::kr::BaseFamilyTag>>>(m, "EffectVariantIndex");
    tyr::bind_index<tyr::Index<Rule<runir::kr::BaseFamilyTag>>>(m, "RuleIndex");
    tyr::bind_index<tyr::Index<Sketch<runir::kr::BaseFamilyTag>>>(m, "SketchIndex");
}

}  // namespace runir::kr::ps::base
