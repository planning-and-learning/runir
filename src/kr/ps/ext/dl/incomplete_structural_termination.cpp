#include "runir/kr/ps/ext/dl/incomplete_structural_termination.hpp"

#include "structural_termination/detail.hpp"

#include <utility>

namespace runir::kr::ps::ext::dl
{

ModuleIncompleteStructuralTerminationResult incomplete_structural_termination(ModuleView module_)
{
    const auto analysis = detail::analyze_module(module_);
    return detail::make_incomplete_result(analysis, runir::kr::ps::detail::incomplete_structural_termination(analysis.policy));
}

ModuleProgramIncompleteStructuralTerminationResult incomplete_structural_termination(ModuleProgramView program)
{
    auto result = ModuleProgramIncompleteStructuralTerminationResult {};
    auto modules = std::vector<ModuleView> {};
    for (auto module : program.get_modules())
    {
        modules.push_back(module);
        auto module_result = incomplete_structural_termination(module);
        if (!module_result.is_terminating())
            result.status = IncompleteStructuralTerminationStatus::UNKNOWN;
        result.module_results.push_back(std::move(module_result));
    }

    result.recursive_call_rules = detail::find_recursive_call_rules(modules);
    if (!result.recursive_call_rules.empty())
        result.status = IncompleteStructuralTerminationStatus::UNKNOWN;
    return result;
}

}  // namespace runir::kr::ps::ext::dl
