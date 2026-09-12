#ifndef RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_DETAIL_HPP_
#define RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_DETAIL_HPP_

#include "kr/ps/structural_termination/detail.hpp"
#include "runir/kr/ps/ext/dl/incomplete_structural_termination.hpp"
#include "runir/kr/ps/ext/dl/structural_termination.hpp"

#include <vector>

namespace runir::kr::ps::ext::dl::detail
{

using Analysis = runir::kr::ps::detail::PolicyAnalysis<runir::kr::ExtFamilyTag, Repository>;

Analysis analyze_module(ModuleView module_);

ModuleStructuralTerminationResult make_result(ModuleView module_, const Analysis& analysis, const runir::kr::ps::detail::PolicySieveResult& sieve_result);
ModuleIncompleteStructuralTerminationResult
make_incomplete_result(ModuleView module_, const Analysis& analysis, const runir::kr::ps::detail::IncompletePolicyResult& policy_result);

std::vector<RuleVariantView> find_recursive_call_rules(const std::vector<ModuleView>& modules);

}  // namespace runir::kr::ps::ext::dl::detail

#endif
