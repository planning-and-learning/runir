#include "runir/kr/ps/ext/dl/incomplete_structural_termination.hpp"

#include "runir/kr/ps/ext/repository.hpp"
#include "structural_termination/detail.hpp"

#include <utility>

namespace runir::kr::ps::ext::dl
{

ModuleIncompleteStructuralTerminationResult incomplete_structural_termination(ModuleView module_, bool use_memory_scc_scope)
{
    const auto analysis = detail::analyze_module(module_);
    return detail::make_incomplete_result(module_, analysis, runir::kr::ps::detail::incomplete_structural_termination(analysis.policy, use_memory_scc_scope));
}

ModuleProgramIncompleteStructuralTerminationResult incomplete_structural_termination(ModuleProgramView program, bool use_memory_scc_scope)
{
    auto result = ModuleProgramIncompleteStructuralTerminationResult {};
    for (auto module : program.get_modules())
    {
        auto module_result = incomplete_structural_termination(module, use_memory_scc_scope);
        if (!module_result.is_terminating())
            result.status = IncompleteStructuralTerminationStatus::UNKNOWN;
        result.module_results.push_back(std::move(module_result));
    }

    result.recursive_call_rules = detail::find_recursive_call_rules(program);
    if (!result.recursive_call_rules.empty())
        result.status = IncompleteStructuralTerminationStatus::UNKNOWN;
    return result;
}

}  // namespace runir::kr::ps::ext::dl
