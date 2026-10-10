#include "bindings.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"

#include <runir/kr/ps/ext/repository.hpp>

namespace runir::kr::ps::ext::dl
{

void bind_feature(nb::module_& m, RepositoryBinding& repository)
{
    runir::kr::python::bind_feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>(m, repository, "ConceptFeature");
    runir::kr::python::bind_feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>(m, repository, "RoleFeature");
    runir::kr::python::bind_feature<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>(m, repository, "BooleanFeature");
    runir::kr::python::bind_feature<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>(m, repository, "NumericalFeature");
    runir::kr::python::bind_feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>(m, repository, "QueryFeature");
}

}  // namespace runir::kr::ps::ext::dl
