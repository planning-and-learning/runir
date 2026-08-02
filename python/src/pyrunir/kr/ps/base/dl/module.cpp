#include "module.hpp"

#include "bindings.hpp"

#include <runir/kr/ps/base/repository.hpp>

namespace runir::kr::ps::base::dl
{

void bind_module_definitions(nb::module_& m, RepositoryBinding& repository)
{
    bind_boolean_feature(m, repository);
    bind_numerical_feature(m, repository);
    runir::kr::ps::base::dl::bind_condition(m, repository);
    runir::kr::ps::base::dl::bind_effect(m, repository);
    bind_evaluation_contexts(m);
    bind_parser(m);
    bind_sketch_factory(m);
    bind_structural_termination(m);
}

}  // namespace runir::kr::ps::base::dl
