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
    using runir::kr::DlTag;
    using runir::kr::dl::BooleanTag;
    using runir::kr::dl::NumericalTag;
    using namespace runir::kr::ps::dl;
    urd::python::bind_index<runir::kr::ps::ConcreteCondition<Family, DlTag, BooleanTag, Positive>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteCondition<Family, DlTag, BooleanTag, Positive>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteCondition<Family, DlTag, BooleanTag, Negative>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteCondition<Family, DlTag, BooleanTag, Negative>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteCondition<Family, DlTag, NumericalTag, EqualZero>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteCondition<Family, DlTag, NumericalTag, EqualZero>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteCondition<Family, DlTag, NumericalTag, GreaterZero>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteCondition<Family, DlTag, NumericalTag, GreaterZero>>(m);
    bind_observation<runir::kr::ps::ConcreteCondition<Family, DlTag, BooleanTag, Positive>>(m, repository);
    bind_observation<runir::kr::ps::ConcreteCondition<Family, DlTag, BooleanTag, Negative>>(m, repository);
    bind_observation<runir::kr::ps::ConcreteCondition<Family, DlTag, NumericalTag, EqualZero>>(m, repository);
    bind_observation<runir::kr::ps::ConcreteCondition<Family, DlTag, NumericalTag, GreaterZero>>(m, repository);
}

}  // namespace runir::kr::ps::base::dl
