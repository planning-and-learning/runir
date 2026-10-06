#ifndef RUNIR_KR_PS_BASE_DETAIL_RULE_EVALUATORS_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_RULE_EVALUATORS_HPP_

#include "runir/kr/ps/base/detail/rule_evaluation/rule.hpp"
#include "runir/kr/ps/base/evaluation_environment.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <optional>

namespace runir::kr::ps::base::detail
{

template<tyr::TaskKind Kind>
class RuleEvaluators
{
    EvaluationEnvironment<Kind> m_environment;
    SketchView m_sketch;

public:
    RuleEvaluators(runir::kr::TaskContext<Kind>& task, SketchView sketch) : m_environment(task), m_sketch(sketch) {}

    auto& get_environment() noexcept { return m_environment; }
    void begin_source() { m_environment.reset_source(); }

    /// Reuse source evaluations across candidates and target evaluations across rules.
    std::optional<RuleView> matching_rule(const tyr::planning::StateView<Kind>& source, const tyr::planning::StateView<Kind>& target, StopConcept auto&& stop)
    {
        if (source == target)
            return std::nullopt;
        m_environment.reset_target();
        auto context = RuleEvaluationContext<BaseFamilyTag, Kind> { m_environment };
        for (const auto rule : m_sketch.get_rules())
        {
            if (stop())
                return std::nullopt;
            const auto evaluator = RuleEvaluator(rule);
            if (evaluator.matches(context, source, target))
                return evaluator.get_rule();
        }
        return std::nullopt;
    }
};

}  // namespace runir::kr::ps::base::detail

#endif
