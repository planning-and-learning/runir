#include "bindings.hpp"
#include "pyrunir/kr/ps/dl/optimization_bindings.hpp"
#include "runir/kr/uns/declarations.hpp"

#include <runir/kr/dl/detail/repository_instantiations.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <runir/kr/uns/classifier_view.hpp>
#include <runir/kr/uns/classify.hpp>
#include <runir/kr/uns/detail/repository_instantiations.hpp>
#include <runir/kr/uns/formatter.hpp>
#include <runir/kr/uns/repository.hpp>
#include <runir/kr/uns/syntactic_complexity.hpp>
#include <tyr/planning/state_repository.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::uns
{

void bind_classifier(nb::module_& m)
{
    using namespace nb::literals;
    using Family = runir::kr::UnsFamilyTag;
    using View = urd::index_view<Classifier, Repository>;
    urd::python::bind_index<urd::index<Classifier>>(m, "ClassifierIndex");
    auto cls = nb::class_<View>(m, "Classifier")
                   .def("get_index", &View::index)
                   .def("get_features", &View::get_features)
                   .def("get_clauses", &View::get_clauses)
                   .def("get_symbol", &View::get_symbol)
                   .def("syntactic_complexity", [](View view) { return syntactic_complexity(view); });
    ygg::add_print(cls);
    urd::python::add_comparison(cls);
    urd::python::add_hash(cls);
    runir::kr::python::bind_ps_optimize<Family, Repository>(cls);

    m.def(
        "classify",
        [](const ClassifierView& classifier, runir::kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>& context)
        { return runir::kr::uns::classify<tyr::GroundTag>(classifier, context); },
        "classifier"_a,
        "context"_a);
    m.def(
        "classify",
        [](const ClassifierView& classifier, runir::kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>& context)
        { return runir::kr::uns::classify<tyr::LiftedTag>(classifier, context); },
        "classifier"_a,
        "context"_a);
}

}  // namespace runir::kr::uns
