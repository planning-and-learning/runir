#include "module.hpp"

#include "bindings.hpp"
#include "pyrunir/kr/ps/icp/dl/module.hpp"
#include "pyrunir/kr/ps/program_bindings.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/repository.hpp>

namespace runir::kr::ps::icp
{

void bind_module_definitions(nb::module_& m)
{
    auto repository = RepositoryBinding(m, "Repository");
    bind_indexical(m, repository);
    urd::python::bind_index<MemoryState<IcpFamilyTag>>(m);
    urd::python::bind_data<MemoryState<IcpFamilyTag>>(m);
    urd::python::bind_index<ModuleSymbol<IcpFamilyTag>>(m);
    urd::python::bind_data<ModuleSymbol<IcpFamilyTag>>(m);
    urd::python::bind_index<ps::Rule<IcpFamilyTag>>(m);
    urd::python::bind_data<ps::Rule<IcpFamilyTag>>(m);
    runir::kr::python::bind_program_record_views<IcpFamilyTag>(m, repository);
    bind_rule(m, repository);
    bind_module(m, repository);
    bind_program(m, repository);
    runir::kr::python::bind_repository(m, repository);
    bind_program_executor(m);

    auto dl = m.def_submodule("dl");
    runir::kr::ps::icp::dl::bind_module_definitions(dl, repository);
}

}  // namespace runir::kr::ps::icp
