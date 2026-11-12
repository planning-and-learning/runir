#ifndef RUNIR_KR_PS_EXT_DL_INCOMPLETE_STRUCTURAL_TERMINATION_DATA_HPP_
#define RUNIR_KR_PS_EXT_DL_INCOMPLETE_STRUCTURAL_TERMINATION_DATA_HPP_

#include "runir/kr/ps/dl/incomplete_structural_termination_data.hpp"
#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/rule_variant_view.hpp"

#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::ps::ext::dl
{

struct ProgramIncompleteStructuralTerminationResult
{
    runir::kr::ps::dl::IncompleteStructuralTerminationStatus status = runir::kr::ps::dl::IncompleteStructuralTerminationStatus::TERMINATING;
    urd::Store<urd::sequence<runir::kr::ps::dl::IncompleteStructuralTerminationResult<runir::kr::ExtFamilyTag>>> module_results;
    urd::Store<urd::sequence<RuleVariantView>> recursive_call_rules;

    [[nodiscard]] bool is_terminating() const noexcept { return status == runir::kr::ps::dl::IncompleteStructuralTerminationStatus::TERMINATING; }
};

}  // namespace runir::kr::ps::ext::dl

#endif
