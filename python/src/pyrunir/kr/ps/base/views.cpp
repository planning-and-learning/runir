#include "pyrunir/kr/ps/base/module.hpp"

#include <concepts>
#include <nanobind/stl/string.h>
#include <runir/kr/ps/base/compatibility.hpp>
#include <runir/kr/ps/base/dl/evaluation_context.hpp>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/syntactic_complexity.hpp>
#include <string>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::base
{

using namespace nanobind::literals;

namespace
{

template<typename T>
void bind_view(nb::module_& m, const std::string& name)
{
    using View = ygg::View<ygg::Index<T>, Repository>;
    using GroundContext = runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::GroundTag>;
    using LiftedContext = runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::LiftedTag>;

    auto cls = nb::class_<View>(m, name.c_str()).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_hash(cls);

    if constexpr (requires(const View& view) { view.get_variant(); })
        cls.def("get_variant", &View::get_variant);
    if constexpr (requires(const View& view) { view.get_feature(); })
        cls.def("get_feature", &View::get_feature, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_expression(); })
        cls.def("get_expression", &View::get_expression, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_symbol(); })
        cls.def("get_symbol", &View::get_symbol);
    if constexpr (requires(const View& view) { view.get_conditions(); })
        cls.def("get_conditions", &View::get_conditions);
    if constexpr (requires(const View& view) { view.get_effects(); })
        cls.def("get_effects", &View::get_effects);
    if constexpr (requires(const View& view) { view.get_rules(); })
        cls.def("get_rules", &View::get_rules);
    if constexpr (requires(const View& view) { view.template get_features<runir::kr::ps::dl::BooleanFeature>(); })
    {
        cls.def("get_boolean_features", &View::template get_features<runir::kr::ps::dl::BooleanFeature>);
        cls.def("get_numerical_features", &View::template get_features<runir::kr::ps::dl::NumericalFeature>);
    }
    if constexpr (requires(const View& view, GroundContext& context) {
                      { is_compatible_with(view, context) } -> std::same_as<bool>;
                  })
        cls.def("is_compatible_with", [](View view, GroundContext& context) { return is_compatible_with(view, context); }, "context"_a);
    if constexpr (requires(const View& view, LiftedContext& context) {
                      { is_compatible_with(view, context) } -> std::same_as<bool>;
                  })
        cls.def("is_compatible_with", [](View view, LiftedContext& context) { return is_compatible_with(view, context); }, "context"_a);
    if constexpr (requires(const View& view) { runir::kr::ps::base::syntactic_complexity(view); })
        cls.def("syntactic_complexity", [](View view) { return runir::kr::ps::base::syntactic_complexity(view); });
}

}  // namespace

void bind_views(nb::module_& m)
{
    bind_view<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>>(m, "ConditionVariant");
    bind_view<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>(m, "EffectVariant");
    bind_view<runir::kr::ps::base::Rule>(m, "Rule");
    bind_view<runir::kr::ps::base::Sketch>(m, "Sketch");
}

}  // namespace runir::kr::ps::base
