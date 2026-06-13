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
    if constexpr (requires(const View& view) { view.get_symbol(); })
        cls.def("get_symbol", [](const View& view) { return std::string(view.get_symbol()); });
    if constexpr (requires(const View& view) { view.get_description(); })
        cls.def("get_description", [](const View& view) { return std::string(view.get_description()); });
    if constexpr (requires(const View& view) { view.get_expression(); })
        cls.def("get_expression", &View::get_expression);
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
    bind_view<Feature<runir::kr::dl::ConceptTag>>(m, "ConceptFeature");
    bind_view<Feature<runir::kr::ps::dl::BooleanFeature>>(m, "BooleanFeature");
    bind_view<Feature<runir::kr::ps::dl::NumericalFeature>>(m, "NumericalFeature");
    bind_view<Register>(m, "Register");
    bind_view<MemoryState>(m, "MemoryState");
    bind_view<Module>(m, "Module");
    bind_view<ModuleProgram>(m, "ModuleProgram");
    bind_view<ConditionVariant>(m, "ConditionVariant");
    bind_view<EffectVariant>(m, "EffectVariant");
    bind_view<RuleVariant>(m, "RuleVariant");
    bind_view<Rule<LoadTag>>(m, "LoadRule");
    bind_view<Rule<SketchTag>>(m, "SketchRule");
    bind_view<Rule<DoTag>>(m, "DoRule");
    bind_view<Rule<CallTag>>(m, "CallRule");

    // Concrete feature/condition/effect views, reachable transitively through the get_variant()
    // accessors of the Feature/ConditionVariant/EffectVariant views above. Mirrors the repository
    // TypeList in runir/kr/ps/ext/repository.hpp (and ps/base/dl/views.cpp, plus ext-only Concept
    // variants). fmt formatters for all of these live in runir/kr/ps/ext/formatter.hpp.
    bind_view<ConcreteFeature<runir::kr::DlTag, runir::kr::dl::ConceptTag>>(m, "ConcreteConceptFeature");
    bind_view<ConcreteFeature<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>(m, "ConcreteBooleanFeature");
    bind_view<ConcreteFeature<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>(m, "ConcreteNumericalFeature");
    bind_view<ConcreteConditionVariant<runir::kr::DlTag>>(m, "ConcreteConditionVariant");
    bind_view<ConcreteCondition<runir::kr::DlTag, runir::kr::dl::ConceptTag, runir::kr::ps::dl::EqualZero>>(m, "EqualZeroConceptCondition");
    bind_view<ConcreteCondition<runir::kr::DlTag, runir::kr::dl::ConceptTag, runir::kr::ps::dl::GreaterZero>>(m, "GreaterZeroConceptCondition");
    bind_view<ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>(m, "PositiveBooleanCondition");
    bind_view<ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>(m, "NegativeBooleanCondition");
    bind_view<ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>>(m, "EqualZeroNumericalCondition");
    bind_view<ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>(m, "GreaterZeroNumericalCondition");

    bind_view<ConcreteEffectVariant<runir::kr::DlTag>>(m, "ConcreteEffectVariant");
    bind_view<ConcreteEffect<runir::kr::DlTag, runir::kr::dl::ConceptTag, runir::kr::ps::dl::Increases>>(m, "IncreasesConceptEffect");
    bind_view<ConcreteEffect<runir::kr::DlTag, runir::kr::dl::ConceptTag, runir::kr::ps::dl::Decreases>>(m, "DecreasesConceptEffect");
    bind_view<ConcreteEffect<runir::kr::DlTag, runir::kr::dl::ConceptTag, runir::kr::ps::dl::Unchanged>>(m, "UnchangedConceptEffect");
    bind_view<ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>(m, "PositiveBooleanEffect");
    bind_view<ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>(m, "NegativeBooleanEffect");
    bind_view<ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>>(m, "UnchangedBooleanEffect");
    bind_view<ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>>(m, "IncreasesNumericalEffect");
    bind_view<ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>>(m, "DecreasesNumericalEffect");
    bind_view<ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>(m, "UnchangedNumericalEffect");
}

}  // namespace runir::kr::ps::ext
