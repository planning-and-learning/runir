#ifndef RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATION_LOAD_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATION_LOAD_HPP_

#include "runir/kr/ps/icp/detail/rule_evaluation/context.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <utility>

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

    template<runir::kr::dl::semantics::EvaluationPolicyConcept<ExtFamilyTag, Kind> EvaluationPolicy, EmitConcept<ProgramStep<Kind>> Emit, StopConcept Stop>
    bool emit(runir::kr::ps::RuleEvaluationContext<IcpFamilyTag, Kind, void, tyr::planning::StateView<Kind>, EvaluationPolicy>& context,
              ProgramStateView<Kind> source,
              Emit&& output,
              Stop&& stop) const
    {
        if (stop())
            return false;
        const auto memory = source.get_memory_state();
        const auto required_memory = m_rule.get_source();
        if (&memory.get_context() != &required_memory.get_context() || memory != required_memory)
            return true;
        using Registers = runir::kr::dl::semantics::RegisterValues;
        auto& workspace = context.workspace;
        const auto& task = workspace.get_task_context();
        auto& environment = workspace.get_environment();
        auto source_context = context.make_dl_context(source);
        if (!conditions_are_compatible<Kind>(m_rule, source_context))
            return true;
        const auto denotation = evaluate<Kind>(m_rule.get_feature(), source_context);
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
            auto transition = context.make_dl_transition_context(source, target_registers);
            if (!runir::kr::ps::all_compatible<Kind>(m_rule.get_effects(), transition))
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
