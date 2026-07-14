#include "pyrunir/kr/ps/ext/module.hpp"

#include <concepts>
#include <nanobind/stl/string.h>
#include <runir/kr/ps/ext/compatibility.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <string>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext
{

using namespace nanobind::literals;

namespace
{

template<typename T>
void bind_view(nb::module_& m, const std::string& name)
{
    using View = ygg::View<ygg::Index<T>, Repository>;

    auto cls = nb::class_<View>(m, name.c_str()).def("get_index", &View::get_index);
    ygg::add_hash(cls);
    ygg::add_print(cls);

    if constexpr (requires(const View& view) { view.get_name(); })
        cls.def("get_name", &View::get_name);
    if constexpr (requires(const View& view) { view.get_identifier(); })
        cls.def("get_identifier", &View::get_identifier);
    if constexpr (requires(const View& view) { view.get_variant(); })
        cls.def("get_variant", &View::get_variant);
    if constexpr (requires(const View& view) { view.get_symbol(); })
        cls.def("get_symbol", &View::get_symbol);
    if constexpr (requires(const View& view) { view.get_feature(); })
        cls.def("get_feature", &View::get_feature, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_source(); })
        cls.def("get_source", &View::get_source, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_target(); })
        cls.def("get_target", &View::get_target, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_rules(); })
        cls.def("get_rules", &View::get_rules);
    if constexpr (requires(const View& view) { view.get_conditions(); })
        cls.def("get_conditions", &View::get_conditions);
    if constexpr (requires(const View& view) { view.get_effects(); })
        cls.def("get_effects", &View::get_effects);
    if constexpr (requires(const View& view) { view.get_register(); })
        cls.def("get_register", &View::get_register, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_action_name(); })
        cls.def("get_action_name", &View::get_action_name);
    if constexpr (requires(const View& view) { view.get_action_arguments(); })
        cls.def("get_action_arguments", &View::get_action_arguments);
    if constexpr (requires(const View& view) { view.get_callee(); })
        cls.def("get_callee", &View::get_callee, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.template get_arguments<runir::kr::dl::ConceptTag>(); })
    {
        cls.def("get_concept_arguments", &View::template get_arguments<runir::kr::dl::ConceptTag>);
        cls.def("get_role_arguments", &View::template get_arguments<runir::kr::dl::RoleTag>);
        cls.def("get_boolean_arguments", &View::template get_arguments<runir::kr::dl::BooleanTag>);
        cls.def("get_numerical_arguments", &View::template get_arguments<runir::kr::dl::NumericalTag>);
    }
    if constexpr (requires(const View& view) { view.template get_registers<runir::kr::dl::ConceptTag>(); })
    {
        cls.def("get_concept_registers", &View::template get_registers<runir::kr::dl::ConceptTag>);
        cls.def("get_role_registers", &View::template get_registers<runir::kr::dl::RoleTag>);
    }
    if constexpr (requires(const View& view) { view.template get_features<runir::kr::dl::ConceptTag>(); })
    {
        cls.def("get_concept_features", &View::template get_features<runir::kr::dl::ConceptTag>);
        cls.def("get_role_features", &View::template get_features<runir::kr::dl::RoleTag>);
        cls.def("get_boolean_features", &View::template get_features<runir::kr::ps::dl::BooleanFeature>);
        cls.def("get_numerical_features", &View::template get_features<runir::kr::ps::dl::NumericalFeature>);
    }
    if constexpr (requires(const View& view) { view.get_entry_memory_state(); })
        cls.def("get_entry_memory_state", &View::get_entry_memory_state, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_entry_module(); })
        cls.def("get_entry_module", &View::get_entry_module, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_memory_states(); })
        cls.def("get_memory_states", &View::get_memory_states);
    if constexpr (requires(const View& view) { view.get_memory_transitions(); })
        cls.def("get_memory_transitions", &View::get_memory_transitions);
    if constexpr (requires(const View& view) { view.get_modules(); })
        cls.def("get_modules", &View::get_modules);
    if constexpr (std::same_as<T, Rule<CallTag>>)
        cls.def(
            "get_call_arguments",
            [](const View& view)
            {
                nb::list result;
                view.for_each_call_argument([&](auto argument) { result.append(nb::cast(argument)); });
                return result;
            },
            nb::keep_alive<0, 1>());
}

}  // namespace

void bind_views(nb::module_& m)
{
    bind_view<MemoryState>(m, "MemoryState");
    bind_view<ModuleSymbol>(m, "ModuleSymbol");
    bind_view<Module>(m, "Module");
    bind_view<ModuleProgram>(m, "ModuleProgram");
    bind_view<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>(m, "ConditionVariant");
    bind_view<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>(m, "EffectVariant");
    bind_view<RuleVariant>(m, "RuleVariant");
    bind_view<Rule<LoadTag<runir::kr::dl::ConceptTag>>>(m, "ConceptLoadRule");
    bind_view<Rule<LoadTag<runir::kr::dl::RoleTag>>>(m, "RoleLoadRule");
    bind_view<Rule<SketchTag>>(m, "SketchRule");
    bind_view<Rule<DoTag>>(m, "DoRule");
    bind_view<Rule<CallTag>>(m, "CallRule");
}

}  // namespace runir::kr::ps::ext
