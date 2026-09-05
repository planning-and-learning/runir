#include "module.hpp"

#include "bindings.hpp"
#include "pyrunir/kr/uns/dl/module.hpp"

#include <runir/kr/uns/repository.hpp>

namespace runir::kr::uns
{

void bind_module_definitions(nb::module_& m)
{
    auto repository = RepositoryBinding(m, "Repository");
    bind_classifier_literal(m);
    bind_classifier_clause(m);
    bind_classifier(m);
    bind_repository(m, repository);

    auto dl = m.def_submodule("dl");
    runir::kr::uns::dl::bind_module_definitions(dl, repository);
}

}  // namespace runir::kr::uns
