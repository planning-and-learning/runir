#include "detail.hpp"
#include "runir/kr/ps/ext/repository.hpp"

namespace runir::kr::ps::ext::dl::detail
{

ModuleStructuralTerminationResult make_result(ModuleView module_, const Analysis& analysis, const runir::kr::ps::detail::PolicySieveResult& sieve_result)
{
    return runir::kr::ps::detail::materialize_result<ModuleStructuralTerminationResult, ModulePolicyGraph>(
        module_,
        analysis,
        sieve_result,
        [module_](std::uint64_t booleans, std::uint64_t numericals, std::size_t memory_position)
        { return ModulePolicyGraphVertexLabel(booleans, numericals, module_.get_memory_states()[memory_position]); });
}

}  // namespace runir::kr::ps::ext::dl::detail
