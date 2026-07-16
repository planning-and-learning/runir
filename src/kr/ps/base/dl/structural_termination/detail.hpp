#ifndef RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_DETAIL_HPP_
#define RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_DETAIL_HPP_

#include "kr/ps/structural_termination/detail.hpp"
#include "runir/kr/ps/base/dl/incomplete_structural_termination.hpp"
#include "runir/kr/ps/base/dl/structural_termination.hpp"
#include "runir/kr/ps/base/rule_view.hpp"

#include <vector>

namespace runir::kr::ps::base::dl::detail
{

struct SketchAnalysis
{
    std::vector<RuleView> rules;
    runir::kr::ps::detail::QualitativePolicy policy;
};

SketchAnalysis analyze_sketch(SketchView sketch);
StructuralTerminationResult make_result(SketchView sketch, const SketchAnalysis& analysis, const runir::kr::ps::detail::PolicySieveResult& sieve_result);

IncompleteStructuralTerminationResult
make_incomplete_result(SketchView sketch, const SketchAnalysis& analysis, const runir::kr::ps::detail::IncompletePolicyResult& policy_result);

}  // namespace runir::kr::ps::base::dl::detail

#endif
