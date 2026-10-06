#ifndef RUNIR_KR_PS_BASE_DETAIL_RULE_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_RULE_EVALUATION_CONTEXT_HPP_

#include "runir/kr/ps/base/evaluation_environment.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

namespace runir::kr::ps
{

template<tyr::TaskKind Kind, runir::kr::dl::semantics::EvaluationPolicyConcept<BaseFamilyTag, Kind> EvaluationPolicy>
struct RuleEvaluationContext<BaseFamilyTag, Kind, void, tyr::planning::StateView<Kind>, EvaluationPolicy>
{
    using FamilyType = BaseFamilyTag;

    base::EvaluationEnvironment<Kind, EvaluationPolicy>& environment;

    auto make_dl_context(tyr::planning::StateView<Kind> source) { return environment.make_dl_context(source); }

    auto make_dl_transition_context(tyr::planning::StateView<Kind> source, tyr::planning::StateView<Kind> candidate)
    {
        return environment.make_dl_transition_context(source, candidate);
    }
};

}  // namespace runir::kr::ps

#endif
