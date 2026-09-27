#include "module.hpp"

#include "bindings.hpp"

#include <runir/kr/ps/icp/repository.hpp>

namespace runir::kr::ps::icp::dl
{

void bind_module_definitions(nb::module_& m, RepositoryBinding& repository)
{
    runir::kr::ps::icp::dl::bind_feature(m, repository);
    runir::kr::ps::icp::dl::bind_condition(m, repository);
    runir::kr::ps::icp::dl::bind_effect(m, repository);
    bind_parser(m);
}

}  // namespace runir::kr::ps::icp::dl
