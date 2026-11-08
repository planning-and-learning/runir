#include "module.hpp"
#include "pyrunir/module.hpp"

#include "bindings.hpp"

#include <runir/kr/ps/icp/repository.hpp>

namespace runir::kr::ps::icp::dl
{

void bind_module_definitions(nb::module_& m, RepositoryBinding& repository)
{
    bind_features(m, repository);
    bind_conditions(m, repository);
    bind_effects(m, repository);
    bind_parser(m);
}

}  // namespace runir::kr::ps::icp::dl
