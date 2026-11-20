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
    using runir::kr::DlTag;
    using runir::kr::dl::BooleanTag;
    using runir::kr::dl::NumericalTag;
    using namespace runir::kr::ps::dl;
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<Family, DlTag, BooleanTag, Positive>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<Family, DlTag, BooleanTag, Positive>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<Family, DlTag, BooleanTag, Negative>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<Family, DlTag, BooleanTag, Negative>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<Family, DlTag, BooleanTag, Unchanged>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<Family, DlTag, BooleanTag, Unchanged>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<Family, DlTag, NumericalTag, Increases>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<Family, DlTag, NumericalTag, Increases>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<Family, DlTag, NumericalTag, Decreases>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<Family, DlTag, NumericalTag, Decreases>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<Family, DlTag, NumericalTag, Unchanged>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<Family, DlTag, NumericalTag, Unchanged>>(m);
    bind_observation<runir::kr::ps::ConcreteEffect<Family, DlTag, BooleanTag, Positive>>(m, repository);
    bind_observation<runir::kr::ps::ConcreteEffect<Family, DlTag, BooleanTag, Negative>>(m, repository);
    bind_observation<runir::kr::ps::ConcreteEffect<Family, DlTag, BooleanTag, Unchanged>>(m, repository);
    bind_observation<runir::kr::ps::ConcreteEffect<Family, DlTag, NumericalTag, Increases>>(m, repository);
    bind_observation<runir::kr::ps::ConcreteEffect<Family, DlTag, NumericalTag, Decreases>>(m, repository);
    bind_observation<runir::kr::ps::ConcreteEffect<Family, DlTag, NumericalTag, Unchanged>>(m, repository);
}

}  // namespace runir::kr::ps::base::dl
