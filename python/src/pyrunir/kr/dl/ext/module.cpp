#include "module.hpp"

#include "bindings.hpp"
#include "semantics/module.hpp"

#include <runir/kr/dl/repository.hpp>

namespace runir::kr::dl::ext
{

void bind_module_definitions(nb::module_& m)
{
    auto repository = RepositoryBinding(m, "ConstructorRepository");
    bind_argument(m, repository);
    bind_register(m, repository);
    bind_constructor(m);
    bind_parser(m);
    bind_repository(m, repository);

    auto semantics_module = m.def_submodule("semantics");
    bind_semantics_module_definitions(semantics_module);
}

}  // namespace runir::kr::dl::ext
