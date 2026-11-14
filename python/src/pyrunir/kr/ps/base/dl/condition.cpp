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

void bind_condition(nb::module_& m, RepositoryBinding& repository)
{
    using Family = runir::kr::BaseFamilyTag;
    bind_observation<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>(
        m,
        repository,
        "PositiveBooleanCondition");
    bind_observation<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>(
        m,
        repository,
        "NegativeBooleanCondition");
    bind_observation<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::EqualZero>>(
        m,
        repository,
        "EqualZeroNumericalCondition");
    bind_observation<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::GreaterZero>>(
        m,
        repository,
        "GreaterZeroNumericalCondition");
}

}  // namespace runir::kr::ps::base::dl
