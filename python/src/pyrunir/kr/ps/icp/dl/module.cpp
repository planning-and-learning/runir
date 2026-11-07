#include "module.hpp"
#include "pyrunir/module.hpp"

#include "bindings.hpp"

#include "pyrunir/kr/ps/dl/binding_utils.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <yggdrasil/database/syntax/query_repository.hpp>

namespace runir::kr::ps::icp::dl
{

void bind_module_definitions(nb::module_& m, RepositoryBinding& repository)
{
    runir::kr::python::bind_program_features<IcpFamilyTag>(m, repository);
    runir::kr::python::bind_program_conditions<IcpFamilyTag>(m, repository);
    runir::kr::python::bind_program_effects<IcpFamilyTag>(m, repository);
    bind_parser(m);
}

}  // namespace runir::kr::ps::icp::dl
