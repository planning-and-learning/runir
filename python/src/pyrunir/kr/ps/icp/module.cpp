#include "module.hpp"
#include "pyrunir/module.hpp"

#include "bindings.hpp"
#include "pyrunir/kr/ps/icp/dl/module.hpp"

#include "pyrunir/kr/ps/program_bindings.hpp"

#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/repository.hpp>

namespace runir::kr::ps::icp
{

void bind_module_definitions(nb::module_& m)
{
    auto repository = RepositoryBinding(m, "Repository");
    runir::kr::python::bind_named<MemoryState<IcpFamilyTag>>(m, repository, "MemoryState");
    runir::kr::python::bind_named<ModuleSymbol<IcpFamilyTag>>(m, repository, "ModuleSymbol");
    bind_indexical(m, repository);
    bind_rule(m, repository);
    runir::kr::python::bind_rule_variant<IcpFamilyTag>(m, repository);
    bind_module(m, repository);
    bind_program(m, repository);
    runir::kr::python::bind_repository(m, repository);
    bind_program_executor(m);

    auto dl = m.def_submodule("dl");
    runir::kr::ps::icp::dl::bind_module_definitions(dl, repository);
}

}  // namespace runir::kr::ps::icp
