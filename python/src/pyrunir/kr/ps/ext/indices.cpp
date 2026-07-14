#include "pyrunir/kr/ps/ext/module.hpp"

#include <runir/kr/ps/ext/repository.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::ext
{

void bind_indices(nb::module_& m)
{
    ygg::bind_index<ygg::Index<MemoryState>>(m, "MemoryStateIndex");
    ygg::bind_index<ygg::Index<ModuleSymbol>>(m, "ModuleSymbolIndex");
    ygg::bind_index<ygg::Index<Module>>(m, "ModuleIndex");
    ygg::bind_index<ygg::Index<ModuleProgram>>(m, "ModuleProgramIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>>(m, "ConditionVariantIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>>(m, "EffectVariantIndex");
    ygg::bind_index<ygg::Index<RuleVariant>>(m, "RuleVariantIndex");
    ygg::bind_index<ygg::Index<Rule<LoadTag<runir::kr::dl::ConceptTag>>>>(m, "ConceptLoadRuleIndex");
    ygg::bind_index<ygg::Index<Rule<LoadTag<runir::kr::dl::RoleTag>>>>(m, "RoleLoadRuleIndex");
    ygg::bind_index<ygg::Index<Rule<SketchTag>>>(m, "SketchRuleIndex");
    ygg::bind_index<ygg::Index<Rule<DoTag>>>(m, "DoRuleIndex");
    ygg::bind_index<ygg::Index<Rule<CallTag>>>(m, "CallRuleIndex");
}

}  // namespace runir::kr::ps::ext
