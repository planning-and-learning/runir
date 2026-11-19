#ifndef RUNIR_KR_PS_BASE_SUCCESSOR_EXPANDER_DECL_HPP_
#define RUNIR_KR_PS_BASE_SUCCESSOR_EXPANDER_DECL_HPP_

#include "runir/kr/ps/base/detail/rule_evaluators.hpp"
#include "runir/kr/ps/base/sketch_executor_data.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/task_context.hpp"

#include <concepts>
#include <optional>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/node.hpp>

namespace runir::kr::ps::base
{

template<tyr::TaskKind Kind, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy = runir::kr::dl::semantics::DefaultEvaluationPolicy<Kind>>
class SuccessorExpander
{
private:
    runir::kr::TaskContext<Kind>* m_task_context;
    detail::RuleEvaluators<Kind, EvaluationPolicy> m_rules;

public:
    using LabeledNode = tyr::planning::LabeledNode<Kind>;

    SuccessorExpander(runir::kr::TaskContext<Kind>& task_context, SketchView sketch) : m_task_context(&task_context), m_rules(task_context, sketch) {}

    auto& get_environment() noexcept { return m_rules.get_environment(); }

    /// Call emit(successor, rule) for each successor permitted by its first matching rule.
    /// emit returning false or stop returning true ends enumeration; stop is also checked for rejected candidates.
    /// Increment statistics.num_generated for every generated successor, including rejected and duplicate targets.
    /// Return true if enumeration completed, false if stopped.
    /// Callbacks must not reenter this expander or its generator.
    template<std::predicate<LabeledNode, RuleView> Emit, StopConcept Stop>
    bool for_each_successor(const tyr::planning::StateView<Kind>& state, SketchSearchStatistics& statistics, const Emit& emit, const Stop& stop);

    std::optional<RuleView> matching_rule(const tyr::planning::StateView<Kind>& source_state, const tyr::planning::StateView<Kind>& target_state);
};

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class SuccessorExpander<tyr::GroundTag>;
extern template class SuccessorExpander<tyr::LiftedTag>;

#endif

}  // namespace runir::kr::ps::base

#endif
