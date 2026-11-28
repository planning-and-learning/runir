#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_CALL_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_CALL_HPP_

#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/context.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <concepts>
#include <optional>
#include <utility>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
class CallRuleEvaluator
{
    RuleView<CallTag> m_rule;
    RuleVariantView m_variant;
    std::optional<ModuleView> m_callee;

public:
    using RuleTag = CallTag;

    CallRuleEvaluator(RuleView<CallTag> rule, RuleVariantView variant, ProgramView program) :
        m_rule(rule),
        m_variant(variant),
        m_callee(program.find_module(rule.callee()))
    {
    }

    [[nodiscard]] auto get_rule() const noexcept { return m_rule; }

    [[nodiscard]] auto get_variant() const noexcept { return m_variant; }

private:
    static bool arguments_match(ModuleView callee, runir::kr::dl::semantics::CallArgumentsView arguments)
    {
        return arguments.template arguments<runir::kr::dl::ConceptTag>().size() == callee.template arguments<runir::kr::dl::ConceptTag>().size()
               && arguments.template arguments<runir::kr::dl::RoleTag>().size() == callee.template arguments<runir::kr::dl::RoleTag>().size()
               && arguments.template arguments<runir::kr::dl::BooleanTag>().size() == callee.template arguments<runir::kr::dl::BooleanTag>().size()
               && arguments.template arguments<runir::kr::dl::NumericalTag>().size() == callee.template arguments<runir::kr::dl::NumericalTag>().size();
    }

    template<runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy,
             ExecutionStorageConcept<Kind> Storage,
             urd::ViewOf<ProgramState<Kind>> State,
             tyr::planning::StateBuilderViewConcept<Kind> PlanningState>
    auto evaluate_call_arguments(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context,
                                 RuleView<CallTag> rule,
                                 State state,
                                 const PlanningState& planning_state) const
    {
        auto result = (*context.task_context)->dl_builder.template scratch<runir::kr::dl::semantics::CallArguments>();
        auto state_context = urd::visit_view(state.module_state(),
                                             [&](const auto& module_)
                                             { return context.environment->make_call_context(planning_state, module_.arguments(), module_.registers()); });
        for (const auto& argument : rule.arguments())
            std::visit(
                [&]<typename FeatureTag, typename FC>(urd::index_view<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>, FC> feature)
                {
                    const auto denotation = evaluate<Kind>(std::visit([](auto concrete) { return concrete.feature(); }, feature.variant()), state_context);
                    if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
                        result->concept_arguments.push_back(denotation.index());
                    else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
                        result->role_arguments.push_back(denotation.index());
                    else if constexpr (std::same_as<FeatureTag, runir::kr::dl::BooleanTag>)
                        result->boolean_arguments.push_back(denotation.index());
                    else
                        result->numerical_arguments.push_back(denotation.index());
                },
                argument);
        return (*context.task_context)->dl_denotation_repository->insert(*result).first;
    }

public:
    template<runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy,
             ExecutionStorageConcept<Kind> Storage,
             EmitConcept<ProgramStep<ExtFamilyTag, Kind, StorageStoredState<Storage>>> Emit,
             StopConcept Stop,
             ExecutionStateViewConcept<Storage> State,
             tyr::planning::StateBuilderViewConcept<Kind> PlanningState>
    bool
    emit(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context, State state, const Emit& emit, const Stop& stop) const
    {
        const auto& planning_state = context.planning_state;
        const auto rule = m_rule;
        const auto rule_variant = m_variant;
        if (!ext::rule_is_applicable(rule, state, planning_state, *context.environment))
            return true;
        auto arguments = evaluate_call_arguments(context, rule, state, planning_state);
        const auto callee = m_callee;
        if (stop())
            return false;
        if (!callee || !arguments_match(*callee, arguments))
            return emit(detail::make_step<Kind, Storage>(ProgramOutcome::MALFORMED_CALL, context.storage->retain(state), *context.task_context));

        auto caller = context.storage->save_caller(state, rule.target());
        auto registers = (*context.task_context)->dl_builder.template scratch<runir::kr::dl::semantics::RegisterValues>();
        registers->concept_values.resize(callee->template registers<runir::kr::dl::ConceptTag>().size());
        registers->role_values.resize(callee->template registers<runir::kr::dl::RoleTag>().size());
        auto target = context.storage->store(planning_state,
                                             *callee,
                                             callee->entry_memory_state(),
                                             context.storage->registers(*registers),
                                             arguments,
                                             std::optional(caller));
        return emit(detail::applied<Kind, Storage>(std::move(target), rule_variant, *context.task_context));
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
