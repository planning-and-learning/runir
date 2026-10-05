#include "bindings.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"

#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/dl/compatibility.hpp>
#include <runir/kr/ps/dl/transition_evaluation_context.hpp>

namespace runir::kr::ps::base::dl
{

namespace
{

template<typename T>
void bind_observation(nb::module_& m, RepositoryBinding& repository, const char* name)
{
    using View = ygg::View<ygg::Index<T>, Repository>;
    using GroundContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>;
    using LiftedContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>;
    runir::kr::python::bind_observation<T>(m, repository, name)
        .def(
            "is_compatible_with",
            [](View value, GroundContext& context) { return runir::kr::ps::is_compatible_with<tyr::GroundTag>(value, context); },
            nb::arg("context"))
        .def("is_compatible_with", [](View value, LiftedContext& context) { return runir::kr::ps::is_compatible_with<tyr::LiftedTag>(value, context); }, nb::arg("context"));
}

}  // namespace

void bind_condition(nb::module_& m, RepositoryBinding& repository)
{
    using Family = runir::kr::BaseFamilyTag;
    using Variant = runir::kr::ps::ConcreteConditionVariant<Family, runir::kr::DlTag>;
    runir::kr::python::bind_variant<Variant>(m, repository, "ConcreteConditionVariant");
    bind_observation<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>(
        m,
        repository,
        "PositiveBooleanCondition");
    bind_observation<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>(
        m,
        repository,
        "NegativeBooleanCondition");
    bind_observation<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>>(
        m,
        repository,
        "EqualZeroNumericalCondition");
    bind_observation<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>(
        m,
        repository,
        "GreaterZeroNumericalCondition");
}

}  // namespace runir::kr::ps::base::dl
