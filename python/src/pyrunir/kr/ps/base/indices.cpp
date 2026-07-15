#include "pyrunir/kr/ps/base/module.hpp"

#include <runir/kr/ps/base/rule_index.hpp>
#include <runir/kr/ps/base/sketch_index.hpp>
#include <runir/kr/ps/condition_index.hpp>
#include <runir/kr/ps/effect_index.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::base
{

void bind_indices(nb::module_& m)
{
    ygg::bind_index<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>>>(m, "ConditionVariantIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>>(m, "EffectVariantIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::base::Rule>>(m, "RuleIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::base::Sketch>>(m, "SketchIndex");
}

}  // namespace runir::kr::ps::base
