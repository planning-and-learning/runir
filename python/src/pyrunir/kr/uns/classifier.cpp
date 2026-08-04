#include "bindings.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/uns/evaluation_context.hpp>
#include <runir/kr/uns/classifier_index.hpp>
#include <runir/kr/uns/classifier_view.hpp>
#include <runir/kr/uns/classify.hpp>
#include <runir/kr/uns/formatter.hpp>
#include <runir/kr/uns/repository.hpp>
#include <runir/kr/uns/syntactic_complexity.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::uns
{

using namespace nanobind::literals;

void bind_classifier(nb::module_& m)
{
    using Family = runir::kr::UnsFamilyTag;
    using View = ygg::View<ygg::Index<Classifier>, Repository>;
    ygg::bind_index<ygg::Index<Classifier>>(m, "ClassifierIndex");
    auto cls = nb::class_<View>(m, "Classifier")
                   .def("get_index", &View::get_index)
                   .def("get_features", &View::get_features)
                   .def("get_clauses", &View::get_clauses)
                   .def("get_symbol", &View::get_symbol)
                   .def("syntactic_complexity", [](View view) { return syntactic_complexity(view); });
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);

    m.def(
        "classify",
        [](ClassifierView classifier, runir::kr::dl::semantics::EvaluationContext<Family, tyr::GroundTag>& context)
        { return runir::kr::uns::classify(classifier, context); },
        "classifier"_a,
        "context"_a);
    m.def(
        "classify",
        [](ClassifierView classifier, runir::kr::dl::semantics::EvaluationContext<Family, tyr::LiftedTag>& context)
        { return runir::kr::uns::classify(classifier, context); },
        "classifier"_a,
        "context"_a);
}

}  // namespace runir::kr::uns
