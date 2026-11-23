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
    urd::python::bind_index<MemoryState<ExtFamilyTag>>(m);
    urd::python::bind_data<MemoryState<ExtFamilyTag>>(m);
    urd::python::bind_index<ModuleSymbol<ExtFamilyTag>>(m);
    urd::python::bind_data<ModuleSymbol<ExtFamilyTag>>(m);
    urd::python::bind_index<ps::Rule<ExtFamilyTag>>(m);
    urd::python::bind_data<ps::Rule<ExtFamilyTag>>(m);
    runir::kr::python::bind_program_record_views<ExtFamilyTag>(m, repository);
    bind_rule(m, repository);
    bind_module(m, repository);
    bind_program(m, repository);
    runir::kr::python::bind_repository(m, repository);
    bind_program_executor(m);

    auto dl = m.def_submodule("dl");
    runir::kr::ps::ext::dl::bind_module_definitions(dl, repository);

    bind_serialization();
}

}  // namespace runir::kr::ps::ext
