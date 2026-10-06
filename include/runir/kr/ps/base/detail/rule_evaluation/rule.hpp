#ifndef RUNIR_KR_PS_BASE_DETAIL_RULE_EVALUATION_RULE_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_RULE_EVALUATION_RULE_HPP_

#include "runir/kr/ps/base/compatibility.hpp"
#include "runir/kr/ps/base/detail/rule_evaluation/context.hpp"

namespace runir::kr::ps::base::detail
{

/// One bound rule; all mutable evaluation storage belongs to RuleEvaluators.
class RuleEvaluator
{
    RuleView m_rule;

public:
    explicit RuleEvaluator(RuleView rule) : m_rule(rule) {}

    auto get_rule() const noexcept { return m_rule; }

    template<tyr::TaskKind Kind, runir::kr::dl::semantics::EvaluationPolicyConcept<BaseFamilyTag, Kind> EvaluationPolicy>
    bool matches(RuleEvaluationContext<BaseFamilyTag, Kind, void, tyr::planning::StateView<Kind>, EvaluationPolicy>& context,
                 tyr::planning::StateView<Kind> source,
                 tyr::planning::StateView<Kind> target) const
    {
        auto transition = context.make_dl_transition_context(source, target);
        return runir::kr::ps::base::is_compatible_with<Kind>(m_rule, transition);
    }
};

}  // namespace runir::kr::ps::base::detail

#endif
