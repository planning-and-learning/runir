#ifndef RUNIR_KR_PS_BASE_SUCCESSOR_EXPANDER_HPP_
#define RUNIR_KR_PS_BASE_SUCCESSOR_EXPANDER_HPP_

#include "runir/kr/ps/base/compatibility.hpp"
#include "runir/kr/ps/base/evaluation_environment.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/task_context.hpp"

#include <optional>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/node.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>
#include <vector>

namespace runir::kr::ps::base
{

template<tyr::TaskKind Kind>
class SuccessorExpander
{
private:
    runir::kr::TaskContext<Kind>& m_task_context;
    SketchView m_sketch;
    EvaluationEnvironment<Kind> m_environment;

public:
    using LabeledNode = tyr::planning::LabeledNode<Kind>;
    using AcceptedSuccessor = std::pair<LabeledNode, RuleView>;

    SuccessorExpander(runir::kr::TaskContext<Kind>& task_context, SketchView sketch) :
        m_task_context(task_context),
        m_sketch(sketch),
        m_environment(task_context)
    {
    }

    auto& get_environment() noexcept { return m_environment; }

    std::vector<LabeledNode> labeled_successors(const tyr::planning::StateView<Kind>& state)
    {
        auto& search_context = *m_task_context.search_context;
        auto& successor_generator = *search_context.successor_generator;
        const auto node = successor_generator.get_node(*search_context.state_repository, state.get_index());
        return successor_generator.get_labeled_successor_nodes(node, *search_context.state_repository, *search_context.axiom_evaluator);
    }

    std::vector<AcceptedSuccessor> accepted_successors(const tyr::planning::StateView<Kind>& state, const std::vector<LabeledNode>& successors)
    {
        return accepted_successors(state, successors, [] { return false; });
    }

    template<typename Stop>
    std::vector<AcceptedSuccessor> accepted_successors(const tyr::planning::StateView<Kind>& state, const std::vector<LabeledNode>& successors, Stop&& stop)
    {
        m_environment.get_dl_caches().clear(false);
        auto result = std::vector<AcceptedSuccessor> {};
        for (const auto& successor : successors)
        {
            if (stop())
                break;
            if (const auto rule = matching_rule_until(state, successor.node.get_state(), stop))
                result.emplace_back(successor, *rule);
        }
        return result;
    }

    std::optional<RuleView> matching_rule(const tyr::planning::StateView<Kind>& source_state, const tyr::planning::StateView<Kind>& target_state)
    {
        m_environment.get_dl_caches().clear(false);
        return matching_rule_until(source_state, target_state, [] { return false; });
    }

    // The caller clears the source cache once before checking a batch of successors.
    std::optional<RuleView>
    matching_rule_until(const tyr::planning::StateView<Kind>& source_state, const tyr::planning::StateView<Kind>& target_state, auto&& stop)
    {
        if (source_state.get_index() == target_state.get_index())
            return std::nullopt;

        m_environment.get_dl_target_caches().clear(false);
        auto transition_context = m_environment.make_dl_transition_context(source_state, target_state);
        for (auto rule : m_sketch.get_rules())
        {
            if (stop())
                return std::nullopt;
            if (runir::kr::ps::base::is_compatible_with(rule, transition_context))
                return rule;
        }
        return std::nullopt;
    }
};

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class SuccessorExpander<tyr::GroundTag>;
extern template class SuccessorExpander<tyr::LiftedTag>;

#endif

}  // namespace runir::kr::ps::base

#endif
