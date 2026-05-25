#include "pyrunir/kr/ps/ext/module.hpp"

#include <runir/kr/ps/ext/repository.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::ps::ext
{

void bind_indices(nb::module_& m)
{
    tyr::bind_index<tyr::Index<Argument<runir::kr::dl::ConceptTag>>>(m, "ConceptArgumentIndex");
    tyr::bind_index<tyr::Index<Argument<runir::kr::dl::RoleTag>>>(m, "RoleArgumentIndex");
    tyr::bind_index<tyr::Index<Argument<runir::kr::dl::BooleanTag>>>(m, "BooleanArgumentIndex");
    tyr::bind_index<tyr::Index<Argument<runir::kr::dl::NumericalTag>>>(m, "NumericalArgumentIndex");
    tyr::bind_index<tyr::Index<Register>>(m, "RegisterIndex");
    tyr::bind_index<tyr::Index<MemoryState>>(m, "MemoryStateIndex");
    tyr::bind_index<tyr::Index<Module>>(m, "ModuleIndex");
    tyr::bind_index<tyr::Index<ConditionVariant>>(m, "ConditionVariantIndex");
    tyr::bind_index<tyr::Index<EffectVariant>>(m, "EffectVariantIndex");
    tyr::bind_index<tyr::Index<RuleVariant>>(m, "RuleVariantIndex");
    tyr::bind_index<tyr::Index<Rule<LoadTag>>>(m, "LoadRuleIndex");
    tyr::bind_index<tyr::Index<Rule<SketchTag>>>(m, "SketchRuleIndex");
    tyr::bind_index<tyr::Index<Rule<DoTag>>>(m, "DoRuleIndex");
    tyr::bind_index<tyr::Index<Rule<CallTag>>>(m, "CallRuleIndex");
}

}  // namespace runir::kr::ps::ext
