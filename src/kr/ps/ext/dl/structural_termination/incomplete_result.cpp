#include "detail.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"

namespace runir::kr::ps::ext::dl::detail
{

runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::ExtFamilyTag>
make_incomplete_result(ModuleView module_, const Analysis& analysis, const runir::kr::ps::detail::IncompletePolicyResult& policy_result)
{ return runir::kr::ps::detail::materialize_incomplete_result<runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::ExtFamilyTag>>(module_, analysis, policy_result); }

}  // namespace runir::kr::ps::ext::dl::detail
