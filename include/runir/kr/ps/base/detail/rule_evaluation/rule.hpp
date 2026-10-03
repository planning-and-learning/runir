#ifndef RUNIR_KR_PS_BASE_DETAIL_RULE_EVALUATION_RULE_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_RULE_EVALUATION_RULE_HPP_

#include "runir/kr/ps/base/compatibility.hpp"

namespace runir::kr::ps::base::detail
{

/// One bound rule; all mutable evaluation storage belongs to RuleEvaluators.
class RuleEvaluator
{
    RuleView m_rule;

public:
    explicit RuleEvaluator(RuleView rule) : m_rule(rule) {}

    auto get_rule() const noexcept { return m_rule; }

    bool matches(auto& transition) const { return runir::kr::ps::base::is_compatible_with(m_rule, transition); }
};

}  // namespace runir::kr::ps::base::detail

#endif
