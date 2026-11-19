#ifndef RUNIR_KR_PS_BASE_DETAIL_SUCCESSOR_EXPANDER_IMPL_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_SUCCESSOR_EXPANDER_IMPL_HPP_

#include "runir/kr/ps/base/successor_expander_decl.hpp"

#include <functional>
#include <optional>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/node.hpp>
#include <tyr/planning/state_repository.hpp>
#include <tyr/planning/successor_generator_concepts.hpp>

namespace runir::kr::ps::base
{

template<tyr::TaskKind Kind, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
template<std::predicate<tyr::planning::LabeledNode<Kind>, RuleView> Emit, StopConcept Stop>
bool SuccessorExpander<Kind, EvaluationPolicy>::for_each_successor(const tyr::planning::StateView<Kind>& state,
                                                                    SketchSearchStatistics& statistics,
                                                                    const Emit& emit,
                                                                    const Stop& stop)
{
    if (stop())
        return false;

    // All candidates share the source state, so its dynamic features are cached for this expansion.
    m_rules.begin_source();
    auto& search_context = *m_task_context->search_context;
    auto& generator = *search_context.successor_generator;
    // Tyr requires a Node, but policy expansion does not carry a path metric.
    const auto node = tyr::planning::Node<Kind>(state, 0);

    const auto visit_binding = [&](tyr::planning::ActionBindingView<Kind> binding)
    {
        if (stop())
            return false;
        const auto successor = generator.get_successor_node(node, binding, *search_context.state_repository, *search_context.axiom_evaluator);
        ++statistics.num_generated;

        const auto rule = m_rules.matching_rule(state, successor.get_state(), stop);
        if (stop())
            return false;
        if (!rule)
            return true;
        return emit(LabeledNode { generator.materialize_action_binding(binding), successor }, *rule);
    };

    return generator.for_each_borrowed_applicable_action_binding(node, std::ref(visit_binding));
}

template<tyr::TaskKind Kind, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
std::optional<RuleView> SuccessorExpander<Kind, EvaluationPolicy>::matching_rule(const tyr::planning::StateView<Kind>& source_state,
                                                                                  const tyr::planning::StateView<Kind>& target_state)
{
    m_rules.begin_source();
    return m_rules.matching_rule(source_state, target_state, [] { return false; });
}

}  // namespace runir::kr::ps::base

#endif
