#include "pyrunir/kr/uns/module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/semantics/uns/evaluation_context.hpp>
#include <runir/kr/uns/classify.hpp>
#include <runir/kr/uns/formatter.hpp>
#include <runir/kr/uns/repository.hpp>
#include <string>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/ground_task/state_repository.hpp>
#include <tyr/planning/ground_task/state_view.hpp>
#include <tyr/planning/lifted_task/state_repository.hpp>
#include <tyr/planning/lifted_task/state_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::uns
{

using namespace nanobind::literals;

namespace
{

using Family = runir::kr::UnsFamilyTag;

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
    if constexpr (requires(const View& view) { view.get_polarity(); })
        cls.def("get_polarity", &View::get_polarity);
    if constexpr (requires(const View& view) { view.get_literals(); })
        cls.def("get_literals", &View::get_literals);
    if constexpr (requires(const View& view) { view.get_features(); })
        cls.def("get_features", &View::get_features);
    if constexpr (requires(const View& view) { view.get_clauses(); })
        cls.def("get_clauses", &View::get_clauses);
    if constexpr (requires(const View& view) { view.get_symbol(); })
        cls.def("get_symbol", [](const View& view) { return std::string(view.get_symbol()); });
    if constexpr (requires(const View& view) { view.get_description(); })
        cls.def("get_description", [](const View& view) { return std::string(view.get_description()); });
}

}  // namespace

void bind_views(nb::module_& m)
{
    bind_view<runir::kr::uns::Feature>(m, "Feature");
    bind_view<runir::kr::uns::ClassifierLiteral>(m, "ClassifierLiteral");
    bind_view<runir::kr::uns::ClassifierClause>(m, "ClassifierClause");
    bind_view<runir::kr::uns::Classifier>(m, "Classifier");

    m.def(
        "classify",
        [](ClassifierView classifier, runir::kr::dl::semantics::EvaluationContext<Family, tyr::planning::GroundTag>& context)
        { return runir::kr::uns::classify(classifier, context); },
        "classifier"_a,
        "context"_a);
    m.def(
        "classify",
        [](ClassifierView classifier, runir::kr::dl::semantics::EvaluationContext<Family, tyr::planning::LiftedTag>& context)
        { return runir::kr::uns::classify(classifier, context); },
        "classifier"_a,
        "context"_a);
}

}  // namespace runir::kr::uns
