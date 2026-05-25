#ifndef RUNIR_KR_PS_EXT_SKETCH_EXECUTOR_HPP_
#define RUNIR_KR_PS_EXT_SKETCH_EXECUTOR_HPP_

#include "runir/datasets/task_class.hpp"
#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/execution.hpp"
#include "runir/kr/ps/ext/module_view.hpp"

#include <cstddef>
#include <memory>
#include <tyr/planning/algorithms/brfs.hpp>
#include <tyr/planning/algorithms/iw.hpp>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/successor_generator.hpp>
#include <vector>

namespace runir::kr::ps::ext
{

enum class ModuleExecutionStatus
{
    SUCCESS,
    FAILURE,
    NO_APPLICABLE_ACTION,
    MALFORMED_CALL,
    SEARCH_FAILURE,
    OUT_OF_TIME,
    OUT_OF_STATES,
    LOAD_LIMIT_REACHED,
    STEP_LIMIT_REACHED,
};

template<tyr::planning::TaskKind Kind>
struct ModuleExecutionOptions
{
    tyr::planning::brfs::Options<Kind> brfs_options;
    tyr::planning::iw::Options<Kind> iw_options;
    tyr::uint_t max_arity = 0;
    std::size_t max_load_steps = 1024;
    std::size_t max_steps = 1024;
};

template<tyr::planning::TaskKind Kind>
class SketchTransitionGoalStrategy : public tyr::planning::GoalStrategy<Kind>
{
private:
    EvaluationContext<Kind>& m_context;

public:
    explicit SketchTransitionGoalStrategy(EvaluationContext<Kind>& context) : m_context(context) {}

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

inline ModuleExecutionStatus translate_search_status(tyr::planning::SearchStatus status)
{
    switch (status)
    {
        case tyr::planning::SearchStatus::SOLVED:
            return ModuleExecutionStatus::SUCCESS;
        case tyr::planning::SearchStatus::OUT_OF_TIME:
            return ModuleExecutionStatus::OUT_OF_TIME;
        case tyr::planning::SearchStatus::OUT_OF_STATES:
        case tyr::planning::SearchStatus::OUT_OF_MEMORY:
            return ModuleExecutionStatus::OUT_OF_STATES;
        case tyr::planning::SearchStatus::FAILED:
        case tyr::planning::SearchStatus::UNSOLVABLE:
        case tyr::planning::SearchStatus::IN_PROGRESS:
        case tyr::planning::SearchStatus::EXHAUSTED:
        case tyr::planning::SearchStatus::CYCLE:
            return ModuleExecutionStatus::SEARCH_FAILURE;
    }

    return ModuleExecutionStatus::SEARCH_FAILURE;
}

template<tyr::planning::TaskKind Kind>
auto find_sketch_transition_node(const runir::datasets::TaskSearchContext<Kind>& search_context,
                                 EvaluationContext<Kind>& context,
                                 const ModuleExecutionOptions<Kind>& options) -> tyr::planning::SearchResult<Kind>
{
    const auto start_node = search_context.successor_generator->get_node(context.get_state().get_index());
    auto brfs_options = options.brfs_options;
    auto iw_options = options.iw_options;
    auto brfs_solver = tyr::planning::brfs::Solver<Kind> { search_context.task, search_context.successor_generator, brfs_options };

    iw_options.start_node = start_node;
    iw_options.goal_strategy = std::make_shared<SketchTransitionGoalStrategy<Kind>>(context);

    return tyr::planning::iw::find_solution(brfs_solver, options.max_arity, iw_options);
}

template<tyr::planning::TaskKind Kind>
struct ModuleExecutionResults
{
    ModuleExecutionStatus status = ModuleExecutionStatus::SUCCESS;
    tyr::planning::StateView<Kind> state;
    ModuleView module;
    MemoryStateView memory_state;
    std::size_t num_steps = 0;
    std::size_t call_depth = 0;

    bool is_successful() const noexcept { return status == ModuleExecutionStatus::SUCCESS; }
};

template<tyr::planning::TaskKind Kind>
auto execute_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                      ModuleView entry_module,
                      const std::vector<ModuleView>& modules,
                      const ModuleExecutionOptions<Kind>& options = ModuleExecutionOptions<Kind>()) -> ModuleExecutionResults<Kind>
{
    auto dl_builder = runir::kr::dl::semantics::Builder();
    auto dl_denotation_repository_factory = runir::kr::dl::semantics::DenotationRepositoryFactory();
    auto dl_denotation_repository = dl_denotation_repository_factory.create();
    auto context = EvaluationContext<Kind>(search_context.successor_generator->get_initial_node().get_state(),
                                           entry_module,
                                           dl_builder,
                                           dl_denotation_repository,
                                           {},
                                           {},
                                           {},
                                           {},
                                           modules);
    auto goal_strategy = tyr::planning::ConjunctiveGoalStrategy<Kind>(*search_context.task);
    const auto initial_state = context.get_state();
    const auto static_goal_satisfied = goal_strategy.is_static_goal_satisfied(*search_context.task);

    auto finish = [&](ModuleExecutionStatus status) {
        return ModuleExecutionResults<Kind> {
            status, context.get_state(), context.get_module(), context.get_memory_state(), 0, context.get_call_stack().size()
        };
    };

    for (std::size_t step = 0; step < options.max_steps; ++step)
    {
        if (static_goal_satisfied && goal_strategy.is_dynamic_goal_satisfied(initial_state, context.get_state()))
        {
            auto result = finish(ModuleExecutionStatus::SUCCESS);
            result.num_steps = step;
            return result;
        }

        const auto load_status = execute_loads(context, options.max_load_steps);
        if (load_status == LoadExecutionStatus::LOAD_LIMIT_REACHED)
        {
            auto result = finish(ModuleExecutionStatus::LOAD_LIMIT_REACHED);
            result.num_steps = step;
            return result;
        }
        if (load_status == LoadExecutionStatus::EMPTY_DENOTATION)
        {
            auto result = finish(ModuleExecutionStatus::FAILURE);
            result.num_steps = step;
            return result;
        }

        const auto node = search_context.successor_generator->get_node(context.get_state().get_index());
        const auto successors = search_context.successor_generator->get_labeled_successor_nodes(node);
        const auto rule_status = execute_next_immediate_external_rule(context, successors);

        if (rule_status == RuleExecutionStatus::APPLIED)
            continue;

        if (rule_status == RuleExecutionStatus::NO_APPLICABLE_ACTION && context.restore_caller())
            continue;

        if (rule_status == RuleExecutionStatus::NO_APPLICABLE_RULE)
        {
            const auto sketch_status = execute_next_sketch_rule(context, successors);
            if (sketch_status == RuleExecutionStatus::APPLIED)
                continue;

            const auto search_result = find_sketch_transition_node(search_context, context, options);
            if (search_result.status == tyr::planning::SearchStatus::SOLVED && search_result.goal_node)
            {
                if (const auto rule = find_matching_sketch_rule(context, search_result.goal_node->get_state()))
                {
                    context.set_state(search_result.goal_node->get_state());
                    context.set_memory_state(rule->get_target());
                    continue;
                }
            }
            else if (search_result.status != tyr::planning::SearchStatus::EXHAUSTED)
            {
                auto result = finish(translate_search_status(search_result.status));
                result.num_steps = step;
                return result;
            }

            if (context.restore_caller())
                continue;
        }

        auto status = ModuleExecutionStatus::FAILURE;
        if (rule_status == RuleExecutionStatus::NO_APPLICABLE_ACTION)
            status = ModuleExecutionStatus::NO_APPLICABLE_ACTION;
        else if (rule_status == RuleExecutionStatus::MALFORMED_CALL)
            status = ModuleExecutionStatus::MALFORMED_CALL;

        auto result = finish(status);
        result.num_steps = step;
        return result;
    }

    auto result = finish(ModuleExecutionStatus::STEP_LIMIT_REACHED);
    result.num_steps = options.max_steps;
    return result;
}

template<tyr::planning::TaskKind Kind>
auto execute_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                      ModuleView entry_module,
                      const ModuleExecutionOptions<Kind>& options = ModuleExecutionOptions<Kind>()) -> ModuleExecutionResults<Kind>
{
    return execute_solution(search_context, entry_module, std::vector<ModuleView> { entry_module }, options);
}

}  // namespace runir::kr::ps::ext

#endif
