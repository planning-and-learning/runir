#include "module.hpp"

#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/ps/ext/repository.hpp>

namespace runir::kr::ps::ext::dl
{

void bind_module_definitions(nb::module_& m, RepositoryBinding& repository)
{
    bind_features(m, repository);
    bind_conditions(m, repository);
    bind_effects(m, repository);
    bind_module_factory(m);
    bind_parser(m);
    bind_structural_termination(m);
}

}  // namespace runir::kr::ps::ext::dl
