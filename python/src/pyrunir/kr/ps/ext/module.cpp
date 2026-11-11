#include "module.hpp"

#include "bindings.hpp"
#include "pyrunir/kr/ps/ext/dl/module.hpp"
#include "pyrunir/kr/ps/program_bindings.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>

namespace runir::kr::ps::ext
{

void bind_module_definitions(nb::module_& m)
{
    auto repository = RepositoryBinding(m, "Repository");
    runir::kr::python::bind_named<MemoryState<ExtFamilyTag>>(m, repository, "MemoryState");
    runir::kr::python::bind_named<ModuleSymbol<ExtFamilyTag>>(m, repository, "ModuleSymbol");
    bind_rule(m, repository);
    runir::kr::python::bind_rule_variant<ExtFamilyTag>(m, repository);
    bind_module(m, repository);
    bind_program(m, repository);
    runir::kr::python::bind_repository(m, repository);
    bind_program_executor(m);

    auto dl = m.def_submodule("dl");
    runir::kr::ps::ext::dl::bind_module_definitions(dl, repository);
}

}  // namespace runir::kr::ps::ext
