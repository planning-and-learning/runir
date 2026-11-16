#include "bindings.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/ps/detail/repository_instantiations.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::ps::ext::dl
{

// One translation unit per kind: the three instantiations together need about 6 GB to compile.
void bind_conditions(nb::module_& m, RepositoryBinding& repository)
{
    using runir::kr::DlTag;
    using runir::kr::dl::BooleanTag;
    using runir::kr::dl::NumericalTag;
    using namespace runir::kr::ps::dl;
    urd::python::bind_data<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, BooleanTag, Positive>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, BooleanTag, Negative>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, NumericalTag, EqualZero>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, NumericalTag, GreaterZero>>(m);
    runir::kr::python::bind_observation_view<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, BooleanTag, Positive>>(repository, m);
    runir::kr::python::bind_observation_view<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, BooleanTag, Negative>>(repository, m);
    runir::kr::python::bind_observation_view<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, NumericalTag, EqualZero>>(repository, m);
    runir::kr::python::bind_observation_view<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, NumericalTag, GreaterZero>>(repository, m);
}

}  // namespace runir::kr::ps::ext::dl
