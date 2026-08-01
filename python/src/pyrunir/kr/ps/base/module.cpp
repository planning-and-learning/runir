#include "module.hpp"

#include "bindings.hpp"
#include "pyrunir/kr/ps/base/dl/module.hpp"

#include <runir/kr/ps/base/repository.hpp>

namespace runir::kr::ps::base
{

void bind_module_definitions(nb::module_& m)
{
    auto repository = RepositoryBinding(m, "Repository");
    bind_condition(m, repository);
    bind_effect(m, repository);
    bind_rule(m, repository);
    bind_sketch(m, repository);
    bind_repository(m, repository);
    bind_sketch_executor(m);
    bind_syntactic_complexity(m);

    auto dl = m.def_submodule("dl");
    runir::kr::ps::base::dl::bind_module_definitions(dl, repository);
}

}  // namespace runir::kr::ps::base
