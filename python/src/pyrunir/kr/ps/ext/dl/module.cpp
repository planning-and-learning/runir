#include "module.hpp"

#include "bindings.hpp"

#include <runir/kr/ps/ext/repository.hpp>

namespace runir::kr::ps::ext::dl
{

void bind_module_definitions(nb::module_& m, RepositoryBinding& repository)
{
    runir::kr::ps::ext::dl::bind_feature(m, repository);
    runir::kr::ps::ext::dl::bind_condition(m, repository);
    runir::kr::ps::ext::dl::bind_effect(m, repository);
    bind_module_factory(m);
    bind_parser(m);
    bind_structural_termination(m);
}

}  // namespace runir::kr::ps::ext::dl
