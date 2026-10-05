#ifndef RUNIR_KR_PS_BASE_DETAIL_RULE_EVALUATORS_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_RULE_EVALUATORS_HPP_

#include "runir/kr/ps/base/detail/rule_evaluation/rule.hpp"
#include "runir/kr/ps/base/evaluation_environment.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <optional>
#include <unordered_map>
#include <vector>

namespace runir::kr::ps::base::detail
{

template<tyr::TaskKind Kind>
class RuleEvaluators
{
    EvaluationEnvironment<Kind> m_environment;
    std::vector<RuleEvaluator> m_evaluators;
    std::vector<std::size_t> m_schedule;

public:
    RuleEvaluators(runir::kr::TaskContext<Kind>& task, SketchView sketch) : m_environment(task)
    {
        auto slots = std::unordered_map<ygg::uint_t, std::size_t> {};
        for (const auto rule : sketch.get_rules())
        {
            const auto [slot, inserted] = slots.try_emplace(ygg::uint_t(rule.get_index()), m_evaluators.size());
            if (inserted)
                m_evaluators.emplace_back(rule);
            m_schedule.push_back(slot->second);
        }
    }

    auto& get_environment() noexcept { return m_environment; }
    void begin_source() { m_environment.reset_source(); }

    /// Reuse source evaluations across candidates and target evaluations across rules.
    std::optional<RuleView> matching_rule(const tyr::planning::StateView<Kind>& source, const tyr::planning::StateView<Kind>& target, StopConcept auto&& stop)
    {
        if (source == target)
            return std::nullopt;
        m_environment.reset_target();
        auto context = RuleEvaluationContext<BaseFamilyTag, Kind> { m_environment };
        static_assert(MatchingRuleEvaluatorConcept<RuleEvaluator,
                                                   BaseFamilyTag,
                                                   Kind,
                                                   decltype(context),
                                                   tyr::planning::StateView<Kind>,
                                                   tyr::planning::StateView<Kind>>);
        for (const auto slot : m_schedule)
        {
            if (stop())
                return std::nullopt;
            const auto& evaluator = m_evaluators[slot];
            if (evaluator.matches(context, source, target))
                return evaluator.get_rule();
        }
        return std::nullopt;
    }
};

}  // namespace runir::kr::ps::base::detail

#endif
