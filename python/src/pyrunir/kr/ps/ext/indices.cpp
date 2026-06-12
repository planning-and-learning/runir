#include "pyrunir/kr/ps/ext/module.hpp"

#include <runir/kr/ps/ext/repository.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::ext
{

void bind_indices(nb::module_& m)
{
    ygg::bind_index<ygg::Index<Argument<runir::kr::dl::ConceptTag>>>(m, "ConceptArgumentIndex");
    ygg::bind_index<ygg::Index<Argument<runir::kr::dl::RoleTag>>>(m, "RoleArgumentIndex");
    ygg::bind_index<ygg::Index<Argument<runir::kr::dl::BooleanTag>>>(m, "BooleanArgumentIndex");
    ygg::bind_index<ygg::Index<Argument<runir::kr::dl::NumericalTag>>>(m, "NumericalArgumentIndex");
    ygg::bind_index<ygg::Index<Register>>(m, "RegisterIndex");
    ygg::bind_index<ygg::Index<MemoryState>>(m, "MemoryStateIndex");
    ygg::bind_index<ygg::Index<Module>>(m, "ModuleIndex");
    ygg::bind_index<ygg::Index<ModuleProgram>>(m, "ModuleProgramIndex");
    ygg::bind_index<ygg::Index<ConditionVariant>>(m, "ConditionVariantIndex");
    ygg::bind_index<ygg::Index<EffectVariant>>(m, "EffectVariantIndex");
    ygg::bind_index<ygg::Index<RuleVariant>>(m, "RuleVariantIndex");
    ygg::bind_index<ygg::Index<Rule<LoadTag>>>(m, "LoadRuleIndex");
    ygg::bind_index<ygg::Index<Rule<SketchTag>>>(m, "SketchRuleIndex");
    ygg::bind_index<ygg::Index<Rule<DoTag>>>(m, "DoRuleIndex");
    ygg::bind_index<ygg::Index<Rule<CallTag>>>(m, "CallRuleIndex");
}

}  // namespace runir::kr::ps::ext
