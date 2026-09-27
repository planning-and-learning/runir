#include "bindings.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"

#include <runir/kr/ps/icp/repository.hpp>

namespace runir::kr::ps::icp::dl
{

void bind_feature(nb::module_& m, RepositoryBinding& repository)
{
    runir::kr::python::bind_feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>(m, repository, "ConceptFeature");
    runir::kr::python::bind_feature<runir::kr::IcpFamilyTag, runir::kr::dl::RoleTag>(m, repository, "RoleFeature");
    runir::kr::python::bind_feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::BooleanFeature>(m, repository, "BooleanFeature");
    runir::kr::python::bind_feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::NumericalFeature>(m, repository, "NumericalFeature");
    runir::kr::python::bind_feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::QueryFeature>(m, repository, "QueryFeature");
}

}  // namespace runir::kr::ps::icp::dl
