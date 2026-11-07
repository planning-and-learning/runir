#include "module.hpp"
#include "pyrunir/module.hpp"

#include "bindings.hpp"

#include "pyrunir/kr/ps/dl/binding_utils.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <yggdrasil/database/syntax/query_repository.hpp>

namespace runir::kr::ps::ext::dl
{

void bind_module_definitions(nb::module_& m, RepositoryBinding& repository)
{
    runir::kr::python::bind_program_features<ExtFamilyTag>(m, repository);
    runir::kr::python::bind_program_conditions<ExtFamilyTag>(m, repository);
    runir::kr::python::bind_program_effects<ExtFamilyTag>(m, repository);
    bind_module_factory(m);
    bind_parser(m);
    bind_structural_termination(m);
}

}  // namespace runir::kr::ps::ext::dl
