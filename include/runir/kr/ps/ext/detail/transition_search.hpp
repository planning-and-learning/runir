#ifndef RUNIR_KR_PS_EXT_DETAIL_TRANSITION_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_TRANSITION_SEARCH_HPP_

#include "runir/datasets/task_class.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/execution.hpp"

#include <memory>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <tyr/planning/node.hpp>

namespace runir::kr::ps::ext::detail
{

template<tyr::planning::TaskKind Kind>
class ModuleProgramTransitionGoalStrategy : public tyr::planning::GoalStrategy<Kind>
{
private:
    EvaluationContext<Kind>& m_context;

public:
    explicit ModuleProgramTransitionGoalStrategy(EvaluationContext<Kind>& context) : m_context(context) {}

    bool is_static_goal_satisfied(const tyr::planning::Task<Kind>& task) override
    {
        static_cast<void>(task);
        return true;
    }

    bool is_dynamic_goal_satisfied(const tyr::planning::StateView<Kind>& seed_state, const tyr::planning::StateView<Kind>& state) override
    {
        if (seed_state.get_index() == state.get_index())
            return false;

        return find_matching_sketch_rule(m_context, state).has_value();
    }
};

template<tyr::planning::TaskKind Kind>
auto find_module_program_transition_node(const runir::datasets::TaskSearchContext<Kind>& search_context,
                                         EvaluationContext<Kind>& context,
                                         const ModuleExecutionOptions<Kind>& options) -> tyr::planning::SearchResult<Kind>
{
    const auto start_node = search_context.successor_generator->get_node(context.get_state().get_index());
    auto brfs_options = options.brfs_options;
    auto iw_options = options.iw_options;
    auto brfs_solver = tyr::planning::brfs::Solver<Kind> { search_context.task, search_context.successor_generator, brfs_options };

    iw_options.start_node = start_node;
    iw_options.goal_strategy = std::make_shared<ModuleProgramTransitionGoalStrategy<Kind>>(context);

    return tyr::planning::iw::find_solution(brfs_solver, options.max_arity, iw_options);
}

}  // namespace runir::kr::ps::ext::detail

#endif
