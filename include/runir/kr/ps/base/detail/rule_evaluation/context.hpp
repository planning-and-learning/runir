#ifndef RUNIR_KR_PS_BASE_DETAIL_RULE_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_RULE_EVALUATION_CONTEXT_HPP_

#include "runir/kr/ps/base/evaluation_environment.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

namespace runir::kr::ps
{

template<tyr::TaskKind Kind>
struct RuleEvaluationContext<BaseFamilyTag, Kind>
{
    using FamilyType = BaseFamilyTag;
    using KindType = Kind;

    base::EvaluationEnvironment<Kind>& environment;

    auto make_dl_context(tyr::planning::StateView<Kind> source) { return environment.make_dl_context(source); }

    auto make_dl_transition_context(tyr::planning::StateView<Kind> source, tyr::planning::StateView<Kind> candidate)
    {
        return environment.make_dl_transition_context(source, candidate);
    }
};

}  // namespace runir::kr::ps

#endif
