#include "bindings.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"

#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/dl/evaluation.hpp>

namespace runir::kr::ps::base::dl
{

void bind_numerical_feature(nb::module_& m, RepositoryBinding& repository)
{
    using GroundContext = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>;
    auto [feature, concrete] =
        runir::kr::python::bind_feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>(m, repository, "NumericalFeature");
    const auto bind_evaluate = []<typename View>(nb::class_<View>& cls)
    {
        cls.def(
               "evaluate",
               [](View value, GroundContext& context) { return runir::kr::ps::evaluate<tyr::GroundTag>(value, context); },
               nb::arg("context"),
               nb::keep_alive<0, 2>())
            .def(
                "evaluate",
                [](View value, LiftedContext& context) { return runir::kr::ps::evaluate<tyr::LiftedTag>(value, context); },
                nb::arg("context"),
                nb::keep_alive<0, 2>());
    };
    bind_evaluate(feature);
    bind_evaluate(concrete);
}

}  // namespace runir::kr::ps::base::dl
