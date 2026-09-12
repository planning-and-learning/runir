#include "detail.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"

namespace runir::kr::ps::ext::dl::detail
{

ModuleIncompleteStructuralTerminationResult
make_incomplete_result(ModuleView module_, const Analysis& analysis, const runir::kr::ps::detail::IncompletePolicyResult& policy_result)
{
    return runir::kr::ps::detail::materialize_incomplete_result<ModuleIncompleteStructuralTerminationResult>(module_, analysis, policy_result);
}

}  // namespace runir::kr::ps::ext::dl::detail
