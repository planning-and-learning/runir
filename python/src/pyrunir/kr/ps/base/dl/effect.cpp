#include "bindings.hpp"
#include "observation.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/dl/compatibility.hpp>
#include <runir/kr/ps/dl/transition_evaluation_context.hpp>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>

namespace runir::kr::ps::base::dl
{

void bind_effect(nb::module_& m, RepositoryBinding& repository)
{
    using Family = runir::kr::BaseFamilyTag;
    bind_observation<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>(m,
                                                                                                                                      repository,
                                                                                                                                      "PositiveBooleanEffect");
    bind_observation<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>(m,
                                                                                                                                      repository,
                                                                                                                                      "NegativeBooleanEffect");
    bind_observation<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Unchanged>>(
        m,
        repository,
        "UnchangedBooleanEffect");
    bind_observation<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Increases>>(
        m,
        repository,
        "IncreasesNumericalEffect");
    bind_observation<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Decreases>>(
        m,
        repository,
        "DecreasesNumericalEffect");
    bind_observation<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Unchanged>>(
        m,
        repository,
        "UnchangedNumericalEffect");
}

}  // namespace runir::kr::ps::base::dl
