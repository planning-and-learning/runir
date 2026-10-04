#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_CALL_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_CALL_HPP_

#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/execution_storage.hpp"
#include "runir/kr/ps/ext/program_view.hpp"

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
        m_callee(program.find_module(rule.get_callee().get_index()))
    {
    }
    auto rule() const noexcept { return m_rule; }
    auto variant() const noexcept { return m_variant; }

private:
    static bool arguments_match(ModuleView callee, runir::kr::dl::semantics::CallArgumentsView arguments)
    {
        return arguments.template get<runir::kr::dl::ConceptTag>().size() == callee.template get_arguments<runir::kr::dl::ConceptTag>().size()
               && arguments.template get<runir::kr::dl::RoleTag>().size() == callee.template get_arguments<runir::kr::dl::RoleTag>().size()
               && arguments.template get<runir::kr::dl::BooleanTag>().size() == callee.template get_arguments<runir::kr::dl::BooleanTag>().size()
               && arguments.template get<runir::kr::dl::NumericalTag>().size() == callee.template get_arguments<runir::kr::dl::NumericalTag>().size();
    }
    template<typename Context, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    auto evaluate_call_arguments(Context& context, RuleView<CallTag> rule, S state, const PS& planning_state)
    {
        auto result = checkout<runir::kr::dl::semantics::CallArguments>(context.task_context->dl_builder);
        auto state_context =
            context.environment.make_call_context(planning_state, state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        rule.for_each_call_argument(
            [&]<typename FeatureTag, typename FC>(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, FC> argument)
            {
                const auto denotation = evaluate(argument.get_expression(), state_context);
                if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
                    result->concept_arguments.push_back(denotation.get_index());
                else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
                    result->role_arguments.push_back(denotation.get_index());
                else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
                    result->boolean_arguments.push_back(denotation.get_index());
                else
                    result->numerical_arguments.push_back(denotation.get_index());
            });
        return get_or_create(*context.task_context->dl_denotation_repository, *result).first;
    }

public:
    template<typename Context, typename Emit, typename Stop, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool emit(Context& context, S state, const PS& planning_state, Emit&& emit, Stop&& stop)
    {
        const auto rule = m_rule;
        const auto rule_variant = m_variant;
        if (!ext::rule_is_applicable(rule, state, planning_state, context.environment))
            return true;
        auto arguments = evaluate_call_arguments(context, rule, state, planning_state);
        const auto callee = m_callee;
        if (stop())
            return false;
        if (!callee || !arguments_match(*callee, arguments))
            return emit(detail::make_step(ProgramOutcome::MALFORMED_CALL, context.storage.retain(state), context.task_context));

        auto caller = context.storage.save_caller(state, rule.get_target());
        auto registers = checkout<runir::kr::dl::semantics::RegisterValues>(context.task_context->dl_builder);
        registers->concept_values.resize(callee->template get_registers<runir::kr::dl::ConceptTag>().size());
        registers->role_values.resize(callee->template get_registers<runir::kr::dl::RoleTag>().size());
        auto target =
            context.storage.store(planning_state, *callee, callee->get_entry_memory_state(), context.storage.registers(*registers), arguments, caller);
        return emit(detail::applied(std::move(target), rule_variant, context.task_context));
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
