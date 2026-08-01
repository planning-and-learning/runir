#include "module.hpp"

#include "bindings.hpp"
#include "pyrunir/kr/ps/ext/dl/module.hpp"

#include <runir/kr/ps/ext/repository.hpp>

namespace runir::kr::ps::ext
{

void bind_module_definitions(nb::module_& m)
{
    auto repository = RepositoryBinding(m, "Repository");
    bind_memory_state(m, repository);
    bind_module_symbol(m, repository);
    bind_condition(m, repository);
    bind_effect(m, repository);
    bind_rule(m, repository);
    bind_rule_variant(m, repository);
    bind_module(m, repository);
    bind_module_program(m, repository);
    bind_repository(m, repository);
    bind_module_program_executor(m);

    auto dl = m.def_submodule("dl");
    runir::kr::ps::ext::dl::bind_module_definitions(dl, repository);
}

}  // namespace runir::kr::ps::ext
