#include "bindings.hpp"
#include "observation.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/database/syntax/query_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/dl/compatibility.hpp>
#include <runir/kr/ps/dl/transition_evaluation_context.hpp>

namespace runir::kr::ps::base::dl
{

void bind_effect(nb::module_& m, RepositoryBinding& repository)
{
    using Family = runir::kr::BaseFamilyTag;
    using runir::kr::dl::BooleanTag;
    using runir::kr::dl::NumericalTag;
    using namespace runir::kr::ps::dl;
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<Family, BooleanTag, Positive>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<Family, BooleanTag, Positive>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<Family, BooleanTag, Negative>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<Family, BooleanTag, Negative>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<Family, BooleanTag, Unchanged>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<Family, BooleanTag, Unchanged>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<Family, NumericalTag, Increases>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<Family, NumericalTag, Increases>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<Family, NumericalTag, Decreases>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<Family, NumericalTag, Decreases>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<Family, NumericalTag, Unchanged>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<Family, NumericalTag, Unchanged>>(m);
    bind_observation<runir::kr::ps::ConcreteEffect<Family, BooleanTag, Positive>>(m, repository);
    bind_observation<runir::kr::ps::ConcreteEffect<Family, BooleanTag, Negative>>(m, repository);
    bind_observation<runir::kr::ps::ConcreteEffect<Family, BooleanTag, Unchanged>>(m, repository);
    bind_observation<runir::kr::ps::ConcreteEffect<Family, NumericalTag, Increases>>(m, repository);
    bind_observation<runir::kr::ps::ConcreteEffect<Family, NumericalTag, Decreases>>(m, repository);
    bind_observation<runir::kr::ps::ConcreteEffect<Family, NumericalTag, Unchanged>>(m, repository);
}

}  // namespace runir::kr::ps::base::dl
