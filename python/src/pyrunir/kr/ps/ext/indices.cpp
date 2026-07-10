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
    ygg::bind_index<ygg::Index<Register<runir::kr::dl::ConceptTag>>>(m, "ConceptRegisterIndex");
    ygg::bind_index<ygg::Index<Register<runir::kr::dl::RoleTag>>>(m, "RoleRegisterIndex");
    ygg::bind_index<ygg::Index<MemoryState>>(m, "MemoryStateIndex");
    ygg::bind_index<ygg::Index<Module>>(m, "ModuleIndex");
    ygg::bind_index<ygg::Index<ModuleProgram>>(m, "ModuleProgramIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>>(m, "ConditionVariantIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>>(m, "EffectVariantIndex");
    ygg::bind_index<ygg::Index<RuleVariant>>(m, "RuleVariantIndex");
    ygg::bind_index<ygg::Index<Rule<LoadTag, runir::kr::dl::ConceptTag>>>(m, "ConceptLoadRuleIndex");
    ygg::bind_index<ygg::Index<Rule<LoadTag, runir::kr::dl::RoleTag>>>(m, "RoleLoadRuleIndex");
    ygg::bind_index<ygg::Index<Rule<SketchTag>>>(m, "SketchRuleIndex");
    ygg::bind_index<ygg::Index<Rule<DoTag>>>(m, "DoRuleIndex");
    ygg::bind_index<ygg::Index<Rule<CallTag>>>(m, "CallRuleIndex");

    // Features
    ygg::bind_index<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>>(m, "ConceptFeatureIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>>(m, "RoleFeatureIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>>(m, "BooleanFeatureIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>>(m, "NumericalFeatureIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::ConceptTag>>>(
        m,
        "ConcreteConceptFeatureIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::RoleTag>>>(m,
                                                                                                                                   "ConcreteRoleFeatureIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>>(
        m,
        "ConcreteBooleanFeatureIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>>(
        m,
        "ConcreteNumericalFeatureIndex");

    // Conditions
    ygg::bind_index<ygg::Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>>(m, "ConcreteConditionVariantIndex");
    ygg::bind_index<ygg::Index<
        runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>>(
        m,
        "PositiveBooleanConditionIndex");
    ygg::bind_index<ygg::Index<
        runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>>(
        m,
        "NegativeBooleanConditionIndex");
    ygg::bind_index<ygg::Index<
        runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>>>(
        m,
        "EqualZeroNumericalConditionIndex");
    ygg::bind_index<ygg::Index<
        runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>>(
        m,
        "GreaterZeroNumericalConditionIndex");

    // Effects
    ygg::bind_index<ygg::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>>(m, "ConcreteEffectVariantIndex");
    ygg::bind_index<
        ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>>(
        m,
        "PositiveBooleanEffectIndex");
    ygg::bind_index<
        ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>>(
        m,
        "NegativeBooleanEffectIndex");
    ygg::bind_index<
        ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>>>(
        m,
        "UnchangedBooleanEffectIndex");
    ygg::bind_index<ygg::Index<
        runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>>>(
        m,
        "IncreasesNumericalEffectIndex");
    ygg::bind_index<ygg::Index<
        runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>>>(
        m,
        "DecreasesNumericalEffectIndex");
    ygg::bind_index<ygg::Index<
        runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>>(
        m,
        "UnchangedNumericalEffectIndex");
}

}  // namespace runir::kr::ps::ext
