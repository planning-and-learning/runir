#include "pyrunir/kr/ps/ext/dl/module.hpp"

#include <concepts>
#include <nanobind/stl/string.h>
#include <runir/kr/ps/ext/compatibility.hpp>
#include <runir/kr/ps/ext/dl/syntactic_complexity.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/syntactic_complexity.hpp>
#include <string>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext::dl
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
    if constexpr (requires(const View& view) { view.get_expression(); })
        cls.def("get_expression", &View::get_expression, nb::keep_alive<0, 1>());
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
    if constexpr (requires(const View& view) { view.get_concept(); })
        cls.def("get_concept", &View::get_concept, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_register(); })
        cls.def("get_register", &View::get_register, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_action_name(); })
        cls.def("get_action_name", &View::get_action_name);
    if constexpr (requires(const View& view) { view.get_action_arguments(); })
        cls.def("get_action_arguments", &View::get_action_arguments);
    if constexpr (requires(const View& view) { view.get_callee(); })
        cls.def("get_callee", &View::get_callee, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { runir::kr::ps::ext::syntactic_complexity(view); })
        cls.def("syntactic_complexity", [](View view) { return runir::kr::ps::ext::syntactic_complexity(view); });
    else if constexpr (requires(const View& view) { runir::kr::ps::ext::dl::syntactic_complexity(view); })
        cls.def("syntactic_complexity", [](View view) { return runir::kr::ps::ext::dl::syntactic_complexity(view); });
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
}

}  // namespace

void bind_views(nb::module_& m)
{
    bind_view<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>(m, "ConceptFeature");
    bind_view<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>(m, "RoleFeature");
    bind_view<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>(m, "BooleanFeature");
    bind_view<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>(m, "NumericalFeature");
    // Concrete feature/condition/effect views reachable through the wrapper variants. Mirrors the repository
    // TypeList in runir/kr/ps/ext/repository.hpp (and ps/base/dl/views.cpp, plus ext-only Concept
    // and Role variants). fmt formatters for all of these live in runir/kr/ps/ext/formatter.hpp.
    bind_view<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::ConceptTag>>(m, "ConcreteConceptFeature");
    bind_view<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::RoleTag>>(m, "ConcreteRoleFeature");
    bind_view<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>(m, "ConcreteBooleanFeature");
    bind_view<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>(m, "ConcreteNumericalFeature");
    bind_view<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>(m, "ConcreteConditionVariant");
    bind_view<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>(
        m,
        "PositiveBooleanCondition");
    bind_view<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>(
        m,
        "NegativeBooleanCondition");
    bind_view<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>>(
        m,
        "EqualZeroNumericalCondition");
    bind_view<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>(
        m,
        "GreaterZeroNumericalCondition");

    bind_view<runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>(m, "ConcreteEffectVariant");
    bind_view<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>(
        m,
        "PositiveBooleanEffect");
    bind_view<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>(
        m,
        "NegativeBooleanEffect");
    bind_view<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>>(
        m,
        "UnchangedBooleanEffect");
    bind_view<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>>(
        m,
        "IncreasesNumericalEffect");
    bind_view<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>>(
        m,
        "DecreasesNumericalEffect");
    bind_view<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>(
        m,
        "UnchangedNumericalEffect");
}

}  // namespace runir::kr::ps::ext::dl
