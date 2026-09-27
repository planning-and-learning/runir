#include "bindings.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"

#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <runir/kr/ps/dl/evaluation.hpp>
#include <runir/kr/uns/repository.hpp>

namespace runir::kr::uns::dl
{

void bind_boolean_feature(nb::module_& m, RepositoryBinding& repository)
{
    using GroundContext = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag>;
    auto [feature, concrete] = runir::kr::python::bind_feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::BooleanFeature>(m, repository, "BooleanFeature");
    const auto bind_evaluate = []<typename View>(nb::class_<View>& cls)
    {
        cls.def(
               "evaluate",
               [](View value, GroundContext& context) { return runir::kr::ps::evaluate(value, context); },
               nb::arg("context"),
               nb::keep_alive<0, 2>())
            .def(
                "evaluate",
                [](View value, LiftedContext& context) { return runir::kr::ps::evaluate(value, context); },
                nb::arg("context"),
                nb::keep_alive<0, 2>());
    };
    bind_evaluate(feature);
    bind_evaluate(concrete);
}

}  // namespace runir::kr::uns::dl
