#ifndef RUNIR_KR_PS_ICP_SUCCESSOR_EXPANDER_HPP_
#define RUNIR_KR_PS_ICP_SUCCESSOR_EXPANDER_HPP_

#include "runir/kr/ps/icp/detail/rule_evaluators.hpp"

namespace runir::kr::ps::icp
{

template<tyr::TaskKind Kind, runir::kr::dl::semantics::EvaluationPolicyConcept<ExtFamilyTag, Kind> EvaluationPolicy = runir::kr::dl::semantics::DefaultEvaluationPolicy<ExtFamilyTag, Kind>>
class SuccessorExpander
{
    detail::RuleEvaluators<Kind, EvaluationPolicy> m_rules;

    void validate(tyr::planning::StateView<Kind> state) const
    {
        if (state.get_state_repository().get() != get_task_context()->search_context->state_repository.get())
            throw std::invalid_argument("ICP requires a planning state from the selected task.");
    }
    void validate(ProgramStateView<Kind> state) const
    {
        if (&state.get_context() != get_task_context()->icp_execution_repository.get() || state.get_program() != m_rules.get_program())
            throw std::invalid_argument("ICP requires an execution state from the selected task and program.");
    }

public:
    using Step = detail::ProgramStep<Kind>;

    SuccessorExpander(TaskContextPtr<Kind> task, ProgramView program) : m_rules(std::move(task), program) {}

    const auto& get_task_context() const noexcept { return m_rules.get_task_context(); }

    ProgramStateView<Kind> initial_state(tyr::planning::StateView<Kind> state)
    {
        validate(state);
        return m_rules.initial_state(state);
    }

    /// Natural rule order for greedy execution; grouped action enumeration for universal search.
    /// Callbacks must not reenter this expander or the planning successor generator.
    template<typename Emit, typename Stop>
    bool for_each_successor(ProgramStateView<Kind> state, ProgramSearchStatistics& statistics, Emit&& emit, Stop&& stop, bool grouped = false)
    {
        validate(state);
        const auto output = [&](Step step)
        {
            statistics.num_generated += step.status == detail::ProgramOutcome::APPLIED;
            return emit(std::move(step));
        };
        return m_rules.for_each_successor(state, output, std::forward<Stop>(stop), grouped);
    }
};

}  // namespace runir::kr::ps::icp

#endif
