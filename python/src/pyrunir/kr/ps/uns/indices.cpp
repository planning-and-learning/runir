#include "pyrunir/kr/ps/uns/module.hpp"

#include <runir/kr/ps/uns/repository.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::uns
{

void bind_indices(nb::module_& m)
{
    ygg::bind_index<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::UnsFamilyTag>>>(m, "ConditionVariantIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::UnsFamilyTag>>>(m, "EffectVariantIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::uns::Rule>>(m, "RuleIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::uns::Sketch>>(m, "SketchIndex");
}

}  // namespace runir::kr::ps::uns
