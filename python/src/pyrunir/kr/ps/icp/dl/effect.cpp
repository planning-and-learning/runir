#include "bindings.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/ps/detail/repository_instantiations.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::ps::icp::dl
{

// One translation unit per kind: the three instantiations together need about 6 GB to compile.
void bind_effects(nb::module_& m, RepositoryBinding& repository)
{
    using runir::kr::DlTag;
    using runir::kr::dl::BooleanTag;
    using runir::kr::dl::NumericalTag;
    using namespace runir::kr::ps::dl;
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<IcpFamilyTag, DlTag, BooleanTag, Positive>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<IcpFamilyTag, DlTag, BooleanTag, Negative>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<IcpFamilyTag, DlTag, BooleanTag, Unchanged>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<IcpFamilyTag, DlTag, NumericalTag, Increases>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<IcpFamilyTag, DlTag, NumericalTag, Decreases>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<IcpFamilyTag, DlTag, NumericalTag, Unchanged>>(m);
    runir::kr::python::bind_observation_view<runir::kr::ps::ConcreteEffect<IcpFamilyTag, DlTag, BooleanTag, Positive>>(repository, m);
    runir::kr::python::bind_observation_view<runir::kr::ps::ConcreteEffect<IcpFamilyTag, DlTag, BooleanTag, Negative>>(repository, m);
    runir::kr::python::bind_observation_view<runir::kr::ps::ConcreteEffect<IcpFamilyTag, DlTag, BooleanTag, Unchanged>>(repository, m);
    runir::kr::python::bind_observation_view<runir::kr::ps::ConcreteEffect<IcpFamilyTag, DlTag, NumericalTag, Increases>>(repository, m);
    runir::kr::python::bind_observation_view<runir::kr::ps::ConcreteEffect<IcpFamilyTag, DlTag, NumericalTag, Decreases>>(repository, m);
    runir::kr::python::bind_observation_view<runir::kr::ps::ConcreteEffect<IcpFamilyTag, DlTag, NumericalTag, Unchanged>>(repository, m);
}

}  // namespace runir::kr::ps::icp::dl
