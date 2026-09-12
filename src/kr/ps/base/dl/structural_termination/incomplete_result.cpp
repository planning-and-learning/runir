#include "detail.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"

namespace runir::kr::ps::base::dl::detail
{

IncompleteStructuralTerminationResult
make_incomplete_result(SketchView sketch, const Analysis& analysis, const runir::kr::ps::detail::IncompletePolicyResult& policy_result)
{
    return runir::kr::ps::detail::materialize_incomplete_result<IncompleteStructuralTerminationResult>(sketch, analysis, policy_result);
}

}  // namespace runir::kr::ps::base::dl::detail
