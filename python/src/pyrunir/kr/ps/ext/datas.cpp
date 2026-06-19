#include "pyrunir/kr/ps/ext/module.hpp"

#include <runir/kr/ps/ext/datas.hpp>
#include <runir/kr/ps/ext/dl/feature_data.hpp>
#include <string>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext
{
namespace
{

template<typename T>
void bind_data(nb::module_& m, const std::string& name)
{
    using Data = ygg::Data<T>;

    auto cls = nb::class_<Data>(m, name.c_str()).def(nb::init<>());

    if constexpr (requires { &Data::index; })
        cls.def_rw("index", &Data::index);
    if constexpr (requires { &Data::name; })
        cls.def_rw("name", &Data::name);
    if constexpr (requires { &Data::identifier; })
        cls.def_rw("identifier", &Data::identifier);
    if constexpr (requires { &Data::value; })
        cls.def_rw("value", &Data::value);
    if constexpr (requires { &Data::feature; })
        cls.def_rw("feature", &Data::feature);
    if constexpr (requires { &Data::symbol; })
        cls.def_rw("symbol", &Data::symbol);
    if constexpr (requires { &Data::description; })
        cls.def_rw("description", &Data::description);
    if constexpr (requires { &Data::source; })
        cls.def_rw("source", &Data::source);
    if constexpr (requires { &Data::target; })
        cls.def_rw("target", &Data::target);
    if constexpr (requires { &Data::conditions; })
        cls.def_rw("conditions", &Data::conditions);
    if constexpr (requires { &Data::effects; })
        cls.def_rw("effects", &Data::effects);
    if constexpr (requires { &Data::load_concept; })
        cls.def_rw("load_concept", &Data::load_concept);
    if constexpr (requires { &Data::reg; })
        cls.def_rw("reg", &Data::reg);
    if constexpr (requires { &Data::action_name; })
        cls.def_rw("action_name", &Data::action_name);
    if constexpr (requires { &Data::arguments; })
        cls.def_rw("arguments", &Data::arguments);
    if constexpr (requires { &Data::callee; })
        cls.def_rw("callee", &Data::callee);
    if constexpr (requires { &Data::concept_arguments; })
        cls.def_rw("concept_arguments", &Data::concept_arguments);
    if constexpr (requires { &Data::role_arguments; })
        cls.def_rw("role_arguments", &Data::role_arguments);
    if constexpr (requires { &Data::boolean_arguments; })
        cls.def_rw("boolean_arguments", &Data::boolean_arguments);
    if constexpr (requires { &Data::numerical_arguments; })
        cls.def_rw("numerical_arguments", &Data::numerical_arguments);
    if constexpr (requires { &Data::registers; })
        cls.def_rw("registers", &Data::registers);
    if constexpr (requires { &Data::concept_features; })
        cls.def_rw("concept_features", &Data::concept_features);
    if constexpr (requires { &Data::boolean_features; })
        cls.def_rw("boolean_features", &Data::boolean_features);
    if constexpr (requires { &Data::numerical_features; })
        cls.def_rw("numerical_features", &Data::numerical_features);
    if constexpr (requires { &Data::entry_memory_state; })
        cls.def_rw("entry_memory_state", &Data::entry_memory_state);
    if constexpr (requires { &Data::entry_module; })
        cls.def_rw("entry_module", &Data::entry_module);
    if constexpr (requires { &Data::memory_states; })
        cls.def_rw("memory_states", &Data::memory_states);
    if constexpr (requires { &Data::memory_transitions; })
        cls.def_rw("memory_transitions", &Data::memory_transitions);
}

}  // namespace

void bind_datas(nb::module_& m)
{
    bind_data<Argument<runir::kr::dl::ConceptTag>>(m, "ConceptArgumentData");
    bind_data<Argument<runir::kr::dl::RoleTag>>(m, "RoleArgumentData");
    bind_data<Argument<runir::kr::dl::BooleanTag>>(m, "BooleanArgumentData");
    bind_data<Argument<runir::kr::dl::NumericalTag>>(m, "NumericalArgumentData");
    bind_data<Register>(m, "RegisterData");
    bind_data<MemoryState>(m, "MemoryStateData");
    bind_data<Module>(m, "ModuleData");
    bind_data<ModuleProgram>(m, "ModuleProgramData");

    bind_data<Feature<runir::kr::dl::ConceptTag>>(m, "ConceptFeatureData");
    bind_data<Feature<runir::kr::ps::dl::BooleanFeature>>(m, "BooleanFeatureData");
    bind_data<Feature<runir::kr::ps::dl::NumericalFeature>>(m, "NumericalFeatureData");
    bind_data<ConcreteFeature<runir::kr::DlTag, runir::kr::dl::ConceptTag>>(m, "DlConceptFeatureData");
    bind_data<ConcreteFeature<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>(m, "DlBooleanFeatureData");
    bind_data<ConcreteFeature<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>(m, "DlNumericalFeatureData");

    bind_data<ConditionVariant>(m, "ConditionVariantData");
    bind_data<ConcreteConditionVariant<runir::kr::DlTag>>(m, "DlConditionVariantData");
    bind_data<ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>(m, "DlBooleanPositiveConditionData");
    bind_data<ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>(m, "DlBooleanNegativeConditionData");
    bind_data<ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>>(m, "DlNumericalEqualZeroConditionData");
    bind_data<ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>(m,
                                                                                                                        "DlNumericalGreaterZeroConditionData");

    bind_data<EffectVariant>(m, "EffectVariantData");
    bind_data<ConcreteEffectVariant<runir::kr::DlTag>>(m, "DlEffectVariantData");
    bind_data<ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>(m, "DlBooleanPositiveEffectData");
    bind_data<ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>(m, "DlBooleanNegativeEffectData");
    bind_data<ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>>(m, "DlBooleanUnchangedEffectData");
    bind_data<ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>>(m, "DlNumericalIncreasesEffectData");
    bind_data<ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>>(m, "DlNumericalDecreasesEffectData");
    bind_data<ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>(m, "DlNumericalUnchangedEffectData");

    bind_data<RuleVariant>(m, "RuleVariantData");
    bind_data<Rule<LoadTag>>(m, "LoadRuleData");
    bind_data<Rule<SketchTag>>(m, "SketchRuleData");
    bind_data<Rule<DoTag>>(m, "DoRuleData");
    bind_data<Rule<CallTag>>(m, "CallRuleData");
}

}  // namespace runir::kr::ps::ext
