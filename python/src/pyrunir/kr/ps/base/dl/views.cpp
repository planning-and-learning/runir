#include "module.hpp"

#include <concepts>
#include <nanobind/stl/string.h>
#include <runir/kr/dl/semantics/evaluation_context.hpp>
#include <runir/kr/ps/base/dl/compatibility.hpp>
#include <runir/kr/ps/base/dl/evaluation.hpp>
#include <runir/kr/ps/base/dl/evaluation_context.hpp>
#include <runir/kr/ps/base/dl/syntactic_complexity.hpp>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/syntactic_complexity.hpp>
#include <runir/kr/ps/repository.hpp>
#include <string>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>
#include <tyr/planning/ground_task/state_repository.hpp>
#include <tyr/planning/ground_task/state_view.hpp>
#include <tyr/planning/lifted_task/state_repository.hpp>
#include <tyr/planning/lifted_task/state_view.hpp>

namespace runir::kr::ps::base::dl
{

using namespace nanobind::literals;

namespace
{

template<tyr::planning::TaskKind Kind>
void bind_evaluation_context(nb::module_& m, const char* name)
{
    using Context = runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, Kind>;

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
    using View = ygg::View<ygg::Index<T>, Repository>;

    auto cls = nb::class_<View>(m, name.c_str()).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_hash(cls);

    if constexpr (requires(const View& view) { view.get_variant(); })
        cls.def("get_variant", &View::get_variant);
    if constexpr (requires(const View& view) { view.get_feature(); })
        cls.def("get_feature", &View::get_feature);
    if constexpr (requires(const View& view) { view.get_expression(); })
        cls.def("get_expression", &View::get_expression);
    if constexpr (requires(const View& view) { view.get_symbol(); })
        cls.def("get_symbol", [](const View& view) { return std::string(view.get_symbol()); });
    if constexpr (requires(const View& view) { view.get_description(); })
        cls.def("get_description", [](const View& view) { return std::string(view.get_description()); });
    if constexpr (requires(const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::GroundTag>& context) {
                      evaluate(view, context);
                  })
        cls.def(
            "evaluate",
            [](const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::GroundTag>& context)
            { return evaluate(view, context); },
            "context"_a);
    if constexpr (requires(const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::LiftedTag>& context) {
                      evaluate(view, context);
                  })
        cls.def(
            "evaluate",
            [](const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::LiftedTag>& context)
            { return evaluate(view, context); },
            "context"_a);
    if constexpr (requires(const View& view, runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::GroundTag>& context) {
                      { is_compatible_with(view, context) } -> std::same_as<bool>;
                  })
        cls.def(
            "is_compatible_with",
            [](const View& view, runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::GroundTag>& context)
            { return is_compatible_with(view, context); },
            "context"_a);
    if constexpr (requires(const View& view, runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::LiftedTag>& context) {
                      { is_compatible_with(view, context) } -> std::same_as<bool>;
                  })
        cls.def(
            "is_compatible_with",
            [](const View& view, runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::LiftedTag>& context)
            { return is_compatible_with(view, context); },
            "context"_a);
    if constexpr (requires(const View& view) { runir::kr::ps::base::syntactic_complexity(view); })
        cls.def("syntactic_complexity", [](const View& view) { return runir::kr::ps::base::syntactic_complexity(view); });
    else if constexpr (requires(const View& view) { runir::kr::ps::base::dl::syntactic_complexity(view); })
        cls.def("syntactic_complexity", [](const View& view) { return runir::kr::ps::base::dl::syntactic_complexity(view); });
}

}  // namespace

void bind_evaluation_contexts(nb::module_& m)
{
    bind_evaluation_context<tyr::planning::GroundTag>(m, "GroundEvaluationContext");
    bind_evaluation_context<tyr::planning::LiftedTag>(m, "LiftedEvaluationContext");
}

void bind_views(nb::module_& m)
{
    bind_view<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>(m, "BooleanFeature");
    bind_view<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>(m, "NumericalFeature");
    bind_view<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>(m, "ConcreteBooleanFeature");
    bind_view<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>(m, "ConcreteNumericalFeature");

    bind_view<runir::kr::ps::ConcreteConditionVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>(m, "ConcreteConditionVariant");
    bind_view<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>(
        m,
        "PositiveBooleanCondition");
    bind_view<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>(
        m,
        "NegativeBooleanCondition");
    bind_view<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>>(
        m,
        "EqualZeroNumericalCondition");
    bind_view<
        runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>(
        m,
        "GreaterZeroNumericalCondition");

    bind_view<runir::kr::ps::ConcreteEffectVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>(m, "ConcreteEffectVariant");
    bind_view<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>(
        m,
        "PositiveBooleanEffect");
    bind_view<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>(
        m,
        "NegativeBooleanEffect");
    bind_view<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>>(
        m,
        "UnchangedBooleanEffect");
    bind_view<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>>(
        m,
        "IncreasesNumericalEffect");
    bind_view<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>>(
        m,
        "DecreasesNumericalEffect");
    bind_view<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>(
        m,
        "UnchangedNumericalEffect");
}

}  // namespace runir::kr::ps::base::dl
