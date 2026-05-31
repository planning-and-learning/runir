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
        cls.def("get_name", [](const View& view) { return std::string(view.get_name()); });
    if constexpr (requires(const View& view) { view.get_identifier(); })
        cls.def("get_identifier", &View::get_identifier);
    if constexpr (requires(const View& view) { view.get_variant(); })
        cls.def("get_variant", &View::get_variant);
    if constexpr (requires(const View& view) { view.get_source(); })
        cls.def("get_source", &View::get_source);
    if constexpr (requires(const View& view) { view.get_target(); })
        cls.def("get_target", &View::get_target);
    if constexpr (requires(const View& view) { view.get_rules(); })
        cls.def("get_rules", &View::get_rules);
    if constexpr (requires(const View& view) { view.get_conditions(); })
        cls.def("get_conditions", &View::get_conditions);
    if constexpr (requires(const View& view) { view.get_effects(); })
        cls.def("get_effects", &View::get_effects);
    if constexpr (requires(const View& view) { view.get_concept(); })
        cls.def("get_concept", &View::get_concept);
    if constexpr (requires(const View& view) { view.get_register(); })
        cls.def("get_register", &View::get_register);
    if constexpr (requires(const View& view) { view.get_action_name(); })
        cls.def("get_action_name", [](const View& view) { return std::string(view.get_action_name()); });
    if constexpr (requires(const View& view) { view.get_action_arguments(); })
        cls.def("get_action_arguments", &View::get_action_arguments);
    if constexpr (requires(const View& view) { view.get_callee(); })
        cls.def("get_callee", &View::get_callee);
    if constexpr (requires(const View& view) { view.get_registers(); })
        cls.def("get_registers", &View::get_registers);
    if constexpr (requires(const View& view) { view.get_entry_memory_state(); })
        cls.def("get_entry_memory_state", &View::get_entry_memory_state);
    if constexpr (requires(const View& view) { view.get_entry_module(); })
        cls.def("get_entry_module", &View::get_entry_module);
    if constexpr (requires(const View& view) { view.get_memory_states(); })
        cls.def("get_memory_states", &View::get_memory_states);
    if constexpr (requires(const View& view) { view.get_memory_transitions(); })
        cls.def("get_memory_transitions", &View::get_memory_transitions);
    if constexpr (requires(const View& view) { view.get_modules(); })
        cls.def("get_modules", &View::get_modules);
}

}  // namespace

void bind_views(nb::module_& m)
{
    bind_view<Argument<runir::kr::dl::ConceptTag>>(m, "ConceptArgument");
    bind_view<Argument<runir::kr::dl::RoleTag>>(m, "RoleArgument");
    bind_view<Argument<runir::kr::dl::BooleanTag>>(m, "BooleanArgument");
    bind_view<Argument<runir::kr::dl::NumericalTag>>(m, "NumericalArgument");
    bind_view<Register>(m, "Register");
    bind_view<MemoryState>(m, "MemoryState");
    bind_view<MemoryTransition>(m, "MemoryTransition");
    bind_view<Module>(m, "Module");
    bind_view<ModuleProgram>(m, "ModuleProgram");
    bind_view<ConditionVariant>(m, "ConditionVariant");
    bind_view<EffectVariant>(m, "EffectVariant");
    bind_view<RuleVariant>(m, "RuleVariant");
    bind_view<Rule<LoadTag>>(m, "LoadRule");
    bind_view<Rule<SketchTag>>(m, "SketchRule");
    bind_view<Rule<DoTag>>(m, "DoRule");
    bind_view<Rule<CallTag>>(m, "CallRule");
}

}  // namespace runir::kr::ps::ext
