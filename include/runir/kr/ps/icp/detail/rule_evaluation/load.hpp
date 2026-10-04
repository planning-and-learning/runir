#ifndef RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATION_LOAD_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATION_LOAD_HPP_

#include "runir/kr/ps/icp/detail/rule_evaluation/workspace.hpp"

namespace runir::kr::ps::icp::detail
{

template<tyr::TaskKind Kind, runir::kr::dl::CategoryTag Category>
class RuleEvaluator<Kind, LoadTag<Category>>
{
    RuleView<LoadTag<Category>> m_rule;
    RuleVariantView m_variant;

public:
    RuleEvaluator(TaskContext<Kind>&, RuleView<LoadTag<Category>> rule, RuleVariantView variant) : m_rule(rule), m_variant(variant) {}

    auto get_rule() const noexcept { return m_rule; }
    bool applicable(auto& context) const { return conditions_are_compatible(m_rule, context); }

    template<typename Emit, typename Stop>
    bool emit(ProgramStateView<Kind> source, RuleEvaluationWorkspace<Kind>& workspace, Emit&& output, Stop&& stop) const
    {
        using Registers = runir::kr::dl::semantics::RegisterValues;
        const auto& task = workspace.get_task_context();
        auto& environment = workspace.get_environment();
        auto context = environment.make_dl_context(source);
        const auto denotation = evaluate(m_rule.get_feature(), context);
        if (denotation.begin() == denotation.end())
        {
            auto step = ProgramStep<Kind>(ProgramOutcome::FAILURE, source, task);
            step.rule = m_variant;
            return output(std::move(step));
        }
        auto registers = checkout<Registers>(task->dl_builder);
        for (const auto value : denotation)
        {
            if (stop())
                return false;
            runir::kr::dl::semantics::assign(*registers, source.get_registers());
            runir::kr::dl::semantics::assign_register(*registers, m_rule.get_register().get_identifier(), value);
            const auto target_registers = insert(*task->dl_denotation_repository, *registers).first;
            environment.reset_target();
            auto transition = environment.make_dl_transition_context(source.get_state(), source.get_state(), source.get_registers(), target_registers);
            if (!runir::kr::ps::all_compatible(m_rule.get_effects(), transition))
                continue;
            const auto histories = workspace.update_histories(source, transition, std::nullopt, stop);
            if (!histories)
                continue;
            auto target = source.get_data();
            target.memory_state = m_rule.get_target().get_index();
            target.registers = target_registers.get_index();
            target.histories = histories->get_index();
            auto step = ProgramStep<Kind>(ProgramOutcome::APPLIED, workspace.intern(target), task);
            step.rule = m_variant;
            if (!output(std::move(step)))
                return false;
        }
        return !stop();
    }
};

}  // namespace runir::kr::ps::icp::detail

#endif
