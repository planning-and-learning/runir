#include "module.hpp"

#include <concepts>
#include <nanobind/stl/string.h>
#include <runir/kr/gp/dl/evaluation_context.hpp>
#include <runir/kr/gp/formatter.hpp>
#include <runir/kr/gp/repository.hpp>
#include <runir/kr/gp/syntactic_complexity.hpp>
#include <string>
#include <tyr/common/python/bindings.hpp>
#include <tyr/common/python/type_casters.hpp>

namespace runir::kr::gp
{

using namespace nanobind::literals;

namespace
{

template<typename T>
void bind_view(nb::module_& m, const std::string& name)
{
    using View = tyr::View<tyr::Index<T>, Repository>;

    auto cls = nb::class_<View>(m, name.c_str()).def("get_index", &View::get_index);
    tyr::add_print(cls);
    tyr::add_hash(cls);

    if constexpr (requires(const View& view) { view.get_variant(); })
        cls.def("get_variant", &View::get_variant);
    if constexpr (requires(const View& view) { view.get_feature(); })
        cls.def("get_feature", &View::get_feature);
    if constexpr (requires(const View& view) { view.get_symbol(); })
        cls.def("get_symbol", [](const View& view) { return std::string(view.get_symbol()); });
    if constexpr (requires(const View& view) { view.get_description(); })
        cls.def("get_description", [](const View& view) { return std::string(view.get_description()); });
    if constexpr (requires(const View& view) { view.get_conditions(); })
        cls.def("get_conditions", &View::get_conditions);
    if constexpr (requires(const View& view) { view.get_effects(); })
        cls.def("get_effects", &View::get_effects);
    if constexpr (requires(const View& view) { view.get_rules(); })
        cls.def("get_rules", &View::get_rules);
    if constexpr (requires(const View& view, runir::kr::gp::dl::EvaluationContext<tyr::planning::GroundTag>& context) {
                      { view.is_compatible_with(context) } -> std::same_as<bool>;
                  })
        cls.def("is_compatible_with", [](const View& view, runir::kr::gp::dl::EvaluationContext<tyr::planning::GroundTag>& context) { return view.is_compatible_with(context); }, "context"_a);
    if constexpr (requires(const View& view, runir::kr::gp::dl::EvaluationContext<tyr::planning::LiftedTag>& context) {
                      { view.is_compatible_with(context) } -> std::same_as<bool>;
                  })
        cls.def("is_compatible_with", [](const View& view, runir::kr::gp::dl::EvaluationContext<tyr::planning::LiftedTag>& context) { return view.is_compatible_with(context); }, "context"_a);
    if constexpr (requires(const View& view) { runir::kr::gp::syntactic_complexity(view); })
        cls.def("syntactic_complexity", [](const View& view) { return runir::kr::gp::syntactic_complexity(view); });
}

}  // namespace

void bind_views(nb::module_& m)
{
    bind_view<ConditionVariant>(m, "ConditionVariant");
    bind_view<EffectVariant>(m, "EffectVariant");
    bind_view<Rule>(m, "Rule");
    bind_view<Policy>(m, "Policy");
}

}  // namespace runir::kr::gp
