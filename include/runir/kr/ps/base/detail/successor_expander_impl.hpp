#ifndef RUNIR_KR_PS_BASE_DETAIL_SUCCESSOR_EXPANDER_IMPL_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_SUCCESSOR_EXPANDER_IMPL_HPP_

#include "runir/datasets/successors.hpp"
#include "runir/kr/ps/base/successor_expander_decl.hpp"

#include <functional>
#include <optional>
#include <tyr/planning/nodes/labeled_node_view.hpp>
#include <tyr/planning/nodes/node_view.hpp>
#include <tyr/planning/states/state_repository.hpp>
#include <tyr/planning/successors/ground/successor_generator.hpp>
#include <tyr/planning/successors/successor_generator_concepts.hpp>

namespace runir::kr::ps::base
{

template<tyr::TaskKind Kind, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
template<std::predicate<runir::datasets::LabeledPlanningNode<Kind>, RuleView> Emit, StopConcept Stop>
bool SuccessorExpander<Kind, EvaluationPolicy>::for_each_successor(const tyr::planning::SharedStateBuilderView<Kind>& state,
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
    // Tyr reads a source from a node; policy expansion carries no path metric.
    const auto node = runir::datasets::node_of<Kind>(state);

    const auto visit_binding = [&](tyr::planning::ActionBindingView<Kind> binding)
    {
        if (stop())
            return false;
        const auto successor = runir::datasets::registered_successor_node<Kind>(search_context, node, binding);
        ++statistics.num_generated;

        const auto rule = m_rules.matching_rule(state, successor.state(), stop);
        if (stop())
            return false;
        if (!rule)
            return true;
        return emit(LabeledNode { runir::datasets::interned_binding(search_context, binding), successor }, *rule);
    };

    return generator.for_each_applicable_action_binding(node, {}, std::ref(visit_binding));
}

template<tyr::TaskKind Kind, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
std::optional<RuleView> SuccessorExpander<Kind, EvaluationPolicy>::matching_rule(const tyr::planning::SharedStateBuilderView<Kind>& source_state,
                                                                                 const tyr::planning::SharedStateBuilderView<Kind>& target_state)
{
    m_rules.begin_source();
    return m_rules.matching_rule(source_state, target_state, [] { return false; });
}

}  // namespace runir::kr::ps::base

#endif
