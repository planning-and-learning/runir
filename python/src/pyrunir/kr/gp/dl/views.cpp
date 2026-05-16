#include "module.hpp"

#include <concepts>
#include <nanobind/stl/string.h>
#include <runir/kr/dl/semantics/evaluation_context.hpp>
#include <runir/kr/gp/dl/evaluation_context.hpp>
#include <runir/kr/gp/dl/syntactic_complexity.hpp>
#include <runir/kr/gp/formatter.hpp>
#include <runir/kr/gp/repository.hpp>
#include <runir/kr/gp/syntactic_complexity.hpp>
#include <string>
#include <tyr/common/python/bindings.hpp>
#include <tyr/common/python/type_casters.hpp>
#include <tyr/planning/ground_task/state_repository.hpp>
#include <tyr/planning/ground_task/state_view.hpp>
#include <tyr/planning/lifted_task/state_repository.hpp>
#include <tyr/planning/lifted_task/state_view.hpp>

namespace runir::kr::gp::dl
{

using namespace nanobind::literals;

namespace
{

template<tyr::planning::TaskKind Kind>
void bind_evaluation_context(nb::module_& m, const char* name)
{
    using Context = runir::kr::gp::dl::EvaluationContext<Kind>;

    nb::class_<Context>(m, name)
        .def(nb::init<tyr::planning::StateView<Kind>,
                      tyr::planning::StateView<Kind>,
                      runir::kr::dl::semantics::Builder&,
                      runir::kr::dl::semantics::DenotationRepository&>(),
             nb::arg("source_state"),
             nb::arg("target_state"),
             nb::arg("builder"),
             nb::arg("denotation_repository"),
             nb::keep_alive<1, 4>(),
             nb::keep_alive<1, 5>())
        .def("get_source_state", &Context::get_source_state, nb::rv_policy::reference_internal)
        .def("get_target_state", &Context::get_target_state, nb::rv_policy::reference_internal);
}

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
    if constexpr (requires(const View& view, runir::kr::dl::semantics::EvaluationContext<tyr::planning::GroundTag>& context) {
                      view.evaluate(context);
                  })
        cls.def("evaluate", [](const View& view, runir::kr::dl::semantics::EvaluationContext<tyr::planning::GroundTag>& context) { return view.evaluate(context); }, "context"_a);
    if constexpr (requires(const View& view, runir::kr::dl::semantics::EvaluationContext<tyr::planning::LiftedTag>& context) {
                      view.evaluate(context);
                  })
        cls.def("evaluate", [](const View& view, runir::kr::dl::semantics::EvaluationContext<tyr::planning::LiftedTag>& context) { return view.evaluate(context); }, "context"_a);
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
    else if constexpr (requires(const View& view) { runir::kr::gp::dl::syntactic_complexity(view); })
        cls.def("syntactic_complexity", [](const View& view) { return runir::kr::gp::dl::syntactic_complexity(view); });
}

}  // namespace

void bind_evaluation_contexts(nb::module_& m)
{
    bind_evaluation_context<tyr::planning::GroundTag>(m, "GroundEvaluationContext");
    bind_evaluation_context<tyr::planning::LiftedTag>(m, "LiftedEvaluationContext");
}

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
