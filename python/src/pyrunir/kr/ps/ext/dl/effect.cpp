#include "bindings.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"

#include <runir/kr/ps/ext/repository.hpp>

namespace runir::kr::ps::ext::dl
{

void bind_effect(nb::module_& m, RepositoryBinding& repository)
{
    using Family = runir::kr::ExtFamilyTag;
    using Variant = runir::kr::ps::ConcreteEffectVariant<Family, runir::kr::DlTag>;
    runir::kr::python::bind_variant<Variant>(m, repository, "ConcreteEffectVariant");
    runir::kr::python::bind_observation<
        runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>(m,
                                                                                                                                 repository,
                                                                                                                                 "PositiveBooleanEffect");
    runir::kr::python::bind_observation<
        runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>(m,
                                                                                                                                 repository,
                                                                                                                                 "NegativeBooleanEffect");
    runir::kr::python::bind_observation<
        runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>>(m,
                                                                                                                                  repository,
                                                                                                                                  "UnchangedBooleanEffect");
    runir::kr::python::bind_observation<
        runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>>(m,
                                                                                                                                    repository,
                                                                                                                                    "IncreasesNumericalEffect");
    runir::kr::python::bind_observation<
        runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>>(m,
                                                                                                                                    repository,
                                                                                                                                    "DecreasesNumericalEffect");
    runir::kr::python::bind_observation<
        runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>(m,
                                                                                                                                    repository,
                                                                                                                                    "UnchangedNumericalEffect");
}

}  // namespace runir::kr::ps::ext::dl
