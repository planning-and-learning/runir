#include "bindings.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/ps/detail/repository_instantiations.hpp"
#include "pyrunir/module.hpp"

#include "pyrunir/kr/ps/dl/binding_utils.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <yggdrasil/database/syntax/query_repository.hpp>

namespace runir::kr::ps::ext::dl
{

// One translation unit per kind: the three instantiations together need about 6 GB to compile.
void bind_conditions(nb::module_& m, RepositoryBinding& repository) { runir::kr::python::bind_program_conditions<ExtFamilyTag>(m, repository); }

}  // namespace runir::kr::ps::ext::dl
