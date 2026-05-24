#include "pyrunir/kr/ps/base/module.hpp"

#include <runir/kr/ps/base/repository.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::ps::base
{

void bind_indices(nb::module_& m)
{
    tyr::bind_index<tyr::Index<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>>>(m, "ConditionVariantIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>>(m, "EffectVariantIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::base::Rule>>(m, "RuleIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::base::Sketch>>(m, "SketchIndex");
}

}  // namespace runir::kr::ps::base
