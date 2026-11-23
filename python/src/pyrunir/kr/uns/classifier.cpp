#include "bindings.hpp"
#include "pyrunir/conversions.hpp"
#include "pyrunir/kr/ps/dl/optimization_bindings.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/uns/declarations.hpp"

#include <runir/kr/database/syntax/query_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <runir/kr/uns/classifier_view.hpp>
#include <runir/kr/uns/classify.hpp>
#include <runir/kr/uns/formatter.hpp>
#include <runir/kr/uns/repository.hpp>
#include <runir/kr/uns/syntactic_complexity.hpp>
#include <tyr/planning/state_repository.hpp>
#include <urd/elements/flat.hpp>
#include <urd/python/bindings.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::uns
{

void bind_classifier(nb::module_& m)
{
    using namespace nb::literals;
    using Family = runir::kr::UnsFamilyTag;
    urd::python::bind_index<Classifier>(m);
    urd::python::bind_data<Classifier>(m);
    urd::python::bind_view<ClassifierView>(m);
    urd::python::bind_builder<runir::kr::uns::Classifier, Repository>(m);
    runir::kr::python::bind_ps_optimize<Family, Repository, ClassifierView>(m);
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
