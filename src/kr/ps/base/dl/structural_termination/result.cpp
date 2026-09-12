#include "detail.hpp"
#include "runir/kr/ps/base/repository.hpp"

namespace runir::kr::ps::base::dl::detail
{

StructuralTerminationResult make_result(SketchView sketch, const Analysis& analysis, const runir::kr::ps::detail::PolicySieveResult& sieve_result)
{
    return runir::kr::ps::detail::materialize_result<StructuralTerminationResult, PolicyGraph>(sketch,
                                                                                               analysis,
                                                                                               sieve_result,
                                                                                               [](std::uint64_t booleans, std::uint64_t numericals, std::size_t)
                                                                                               { return PolicyGraphVertexLabel(booleans, numericals); });
}

}  // namespace runir::kr::ps::base::dl::detail
