#include "bindings.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"

#include <runir/kr/ps/ext/repository.hpp>

namespace runir::kr::ps::ext::dl
{

void bind_condition(nb::module_& m, RepositoryBinding& repository)
{
    using Family = runir::kr::ExtFamilyTag;
    using Variant = runir::kr::ps::ConcreteConditionVariant<Family, runir::kr::DlTag>;
    runir::kr::python::bind_variant<Variant>(m, repository, "ConcreteConditionVariant");
    runir::kr::python::bind_observation<
        runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>(m,
                                                                                                                                    repository,
                                                                                                                                    "PositiveBooleanCondition");
    runir::kr::python::bind_observation<
        runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>(m,
                                                                                                                                    repository,
                                                                                                                                    "NegativeBooleanCondition");
    runir::kr::python::bind_observation<
        runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>>(
        m,
        repository,
        "EqualZeroNumericalCondition");
    runir::kr::python::bind_observation<
        runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>(
        m,
        repository,
        "GreaterZeroNumericalCondition");
}

}  // namespace runir::kr::ps::ext::dl
