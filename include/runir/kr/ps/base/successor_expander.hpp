#ifndef RUNIR_KR_PS_BASE_SUCCESSOR_EXPANDER_HPP_
#define RUNIR_KR_PS_BASE_SUCCESSOR_EXPANDER_HPP_

#include "runir/kr/ps/base/compatibility.hpp"
#include "runir/kr/ps/base/evaluation_environment.hpp"
#include "runir/kr/ps/base/sketch_executor_data.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/task_context.hpp"

#include <functional>
#include <optional>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/node.hpp>
#include <tyr/planning/state_view.hpp>

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

    SuccessorExpander(runir::kr::TaskContext<Kind>& task_context, SketchView sketch) :
        m_task_context(task_context),
        m_sketch(sketch),
        m_environment(task_context)
    {
    }

    auto& get_environment() noexcept { return m_environment; }

    /// Call emit(successor, rule) for each successor permitted by its first matching rule.
    /// emit returning false or stop returning true ends enumeration; stop is also checked for rejected candidates.
    /// Increment statistics.num_generated for every generated successor, including rejected and duplicate targets.
    /// Return true if enumeration completed, false if stopped.
    /// Callbacks must not reenter this expander or its generator.
    template<typename Emit, typename Stop>
    bool for_each_successor(const tyr::planning::StateView<Kind>& state, SketchSearchStatistics& statistics, Emit&& emit, Stop&& stop)
    {
        if (stop())
            return false;

        // All candidates share the source state, so its dynamic features are cached for this expansion.
        m_environment.get_dl_caches().clear(false);
        auto& search_context = *m_task_context.search_context;
        auto& generator = *search_context.successor_generator;
        // Tyr requires a Node, but policy expansion does not carry a path metric.
        const auto node = tyr::planning::Node<Kind>(state, 0);

        const auto visit_binding = [&](tyr::formalism::planning::ActionBindingView binding)
        {
            if (stop())
                return false;
            const auto successor =
                LabeledNode { binding, generator.get_successor_node(node, binding, *search_context.state_repository, *search_context.axiom_evaluator) };
            ++statistics.num_generated;

            const auto rule = matching_rule_until(state, successor.node.get_state(), stop);
            if (stop())
                return false;
            if (!rule)
                return true;
            return emit(successor, *rule);
        };

        return generator.for_each_applicable_action_binding(node, std::ref(visit_binding));
    }

    std::optional<RuleView> matching_rule(const tyr::planning::StateView<Kind>& source_state, const tyr::planning::StateView<Kind>& target_state)
    {
        m_environment.get_dl_caches().clear(false);
        return matching_rule_until(source_state, target_state, [] { return false; });
    }

private:
    // Reuse the prepared source cache; refresh dynamic target features for each candidate.
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
