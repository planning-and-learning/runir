#include "module.hpp"

#include "bindings.hpp"

#include <runir/kr/uns/repository.hpp>

namespace runir::kr::uns::dl
{

void bind_module_definitions(nb::module_& m, RepositoryBinding& repository)
{
    bind_boolean_feature(m, repository);
    bind_parser(m);
    bind_classifier_factory(m);
}

}  // namespace runir::kr::uns::dl
