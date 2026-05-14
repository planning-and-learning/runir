#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/gp/repository.hpp>
#include <string>
#include <tyr/common/python/bindings.hpp>
#include <tyr/common/python/type_casters.hpp>

namespace runir::kr::gp::dl
{

namespace
{

template<typename T>
void bind_view(nb::module_& m, const std::string& name)
{
    using View = tyr::View<tyr::Index<T>, Repository>;

    auto cls = nb::class_<View>(m, name.c_str()).def("get_index", &View::get_index);
    tyr::add_hash(cls);

    if constexpr (requires(const View& view) { view.get_variant(); })
        cls.def("get_variant", &View::get_variant);
    if constexpr (requires(const View& view) { view.get_feature(); })
        cls.def("get_feature", &View::get_feature);
    if constexpr (requires(const View& view) { view.get_symbol(); })
        cls.def("get_symbol", [](const View& view) { return std::string(view.get_symbol()); });
    if constexpr (requires(const View& view) { view.get_description(); })
        cls.def("get_description", [](const View& view) { return std::string(view.get_description()); });
}

}  // namespace

void bind_views(nb::module_& m)
{
    bind_view<Feature<BooleanFeature>>(m, "BooleanFeature");
    bind_view<Feature<NumericalFeature>>(m, "NumericalFeature");
    bind_view<ConcreteFeature<runir::kr::DlTag, BooleanFeature>>(m, "ConcreteBooleanFeature");
    bind_view<ConcreteFeature<runir::kr::DlTag, NumericalFeature>>(m, "ConcreteNumericalFeature");

    bind_view<ConcreteConditionVariant<runir::kr::DlTag>>(m, "ConcreteConditionVariant");
    bind_view<ConcreteCondition<runir::kr::DlTag, BooleanFeature, Positive>>(m, "PositiveBooleanCondition");
    bind_view<ConcreteCondition<runir::kr::DlTag, BooleanFeature, Negative>>(m, "NegativeBooleanCondition");
    bind_view<ConcreteCondition<runir::kr::DlTag, NumericalFeature, EqualZero>>(m, "EqualZeroNumericalCondition");
    bind_view<ConcreteCondition<runir::kr::DlTag, NumericalFeature, GreaterZero>>(m, "GreaterZeroNumericalCondition");

    bind_view<ConcreteEffectVariant<runir::kr::DlTag>>(m, "ConcreteEffectVariant");
    bind_view<ConcreteEffect<runir::kr::DlTag, BooleanFeature, Positive>>(m, "PositiveBooleanEffect");
    bind_view<ConcreteEffect<runir::kr::DlTag, BooleanFeature, Negative>>(m, "NegativeBooleanEffect");
    bind_view<ConcreteEffect<runir::kr::DlTag, BooleanFeature, Unchanged>>(m, "UnchangedBooleanEffect");
    bind_view<ConcreteEffect<runir::kr::DlTag, NumericalFeature, Increases>>(m, "IncreasesNumericalEffect");
    bind_view<ConcreteEffect<runir::kr::DlTag, NumericalFeature, Decreases>>(m, "DecreasesNumericalEffect");
    bind_view<ConcreteEffect<runir::kr::DlTag, NumericalFeature, Unchanged>>(m, "UnchangedNumericalEffect");
}

}  // namespace runir::kr::gp::dl
