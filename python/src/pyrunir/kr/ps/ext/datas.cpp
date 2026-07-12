#include "pyrunir/kr/ps/ext/module.hpp"

#include <runir/kr/ps/ext/datas.hpp>
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
    if constexpr (requires { &Data::source; })
        cls.def_rw("source", &Data::source);
    if constexpr (requires { &Data::target; })
        cls.def_rw("target", &Data::target);
    if constexpr (requires { &Data::conditions; })
        cls.def_rw("conditions", &Data::conditions);
    if constexpr (requires { &Data::effects; })
        cls.def_rw("effects", &Data::effects);
    if constexpr (requires { &Data::load_expression; })
        cls.def_rw("load_expression", &Data::load_expression);
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
    if constexpr (requires { &Data::concept_registers; })
        cls.def_rw("concept_registers", &Data::concept_registers);
    if constexpr (requires { &Data::role_registers; })
        cls.def_rw("role_registers", &Data::role_registers);
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
    bind_data<Register<runir::kr::dl::ConceptTag>>(m, "ConceptRegisterData");
    bind_data<Register<runir::kr::dl::RoleTag>>(m, "RoleRegisterData");
    bind_data<MemoryState>(m, "MemoryStateData");
    bind_data<Module>(m, "ModuleData");
    bind_data<ModuleProgram>(m, "ModuleProgramData");

    bind_data<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>(m, "ConditionVariantData");
    bind_data<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>(m, "EffectVariantData");
    bind_data<RuleVariant>(m, "RuleVariantData");
    bind_data<Rule<LoadTag<runir::kr::dl::ConceptTag>>>(m, "ConceptLoadRuleData");
    bind_data<Rule<LoadTag<runir::kr::dl::RoleTag>>>(m, "RoleLoadRuleData");
    bind_data<Rule<SketchTag>>(m, "SketchRuleData");
    bind_data<Rule<DoTag>>(m, "DoRuleData");
    bind_data<Rule<CallTag>>(m, "CallRuleData");
}

}  // namespace runir::kr::ps::ext
