#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_IMPL_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_IMPL_HPP_

#include "runir/graphs/algorithms.hpp"
#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/execution.hpp"
#include "runir/kr/ps/ext/module_program_executor.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <sstream>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <tyr/planning/node.hpp>
#include <tyr/planning/successor_generator.hpp>
#include <unordered_map>

namespace runir::kr::ps::ext
{
namespace detail
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
struct ModuleExecutionResults
{
    ModuleExecutionStatus status = ModuleExecutionStatus::SUCCESS;
    tyr::planning::StateView<Kind> state;
    ModuleView module;
    MemoryStateView memory_state;
    std::optional<tyr::planning::Plan<Kind>> plan = std::nullopt;
    std::size_t num_steps = 0;
    std::size_t call_depth = 0;
};

template<tyr::planning::TaskKind Kind>
auto execute_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                      ModuleProgramView program,
                      const ModuleExecutionOptions<Kind>& options) -> ModuleExecutionResults<Kind>;

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

template<tyr::planning::TaskKind Kind>
ModuleExecutionOptions<Kind> execution_options(const ModuleProgramSearchOptions<Kind>& options)
{
    auto result = ModuleExecutionOptions<Kind> {};
    result.brfs_options = options.brfs_options;
    result.iw_options = options.iw_options;
    result.max_arity = options.max_arity;
    result.max_load_steps = options.max_load_steps;
    result.max_steps = options.max_steps;
    return result;
}

inline tyr::planning::SearchStatus translate_execution_status(ModuleExecutionStatus status)
{
    switch (status)
    {
        case ModuleExecutionStatus::SUCCESS:
            return tyr::planning::SearchStatus::SOLVED;
        case ModuleExecutionStatus::OUT_OF_TIME:
            return tyr::planning::SearchStatus::OUT_OF_TIME;
        case ModuleExecutionStatus::OUT_OF_STATES:
            return tyr::planning::SearchStatus::OUT_OF_STATES;
        case detail::ModuleExecutionStatus::FAILURE:
        case detail::ModuleExecutionStatus::NO_APPLICABLE_ACTION:
        case detail::ModuleExecutionStatus::MALFORMED_CALL:
        case ModuleExecutionStatus::SEARCH_FAILURE:
        case ModuleExecutionStatus::LOAD_LIMIT_REACHED:
        case ModuleExecutionStatus::STEP_LIMIT_REACHED:
            return tyr::planning::SearchStatus::FAILED;
    }

    return tyr::planning::SearchStatus::FAILED;
}

}  // namespace detail

template<tyr::planning::TaskKind Kind>
auto find_solution(const runir::datasets::TaskSearchContext<Kind>& context,
                   ModuleProgramView program,
                   const ModuleProgramSearchOptions<Kind>& options) -> tyr::planning::SearchResult<Kind>
{
    const auto execution = detail::execute_solution(context, program, detail::execution_options(options));

    auto result = tyr::planning::SearchResult<Kind> {};
    result.status = detail::translate_execution_status(execution.status);
    if (execution.status == detail::ModuleExecutionStatus::SUCCESS)
    {
        result.goal_node = context.successor_generator->get_node(execution.state.get_index());
        result.plan = execution.plan;
    }
    return result;
}

namespace detail
{

template<typename Graph>
class CycleVisitor : public graphs::bgl::TraversalVisitor<Graph>
{
private:
    const Graph& m_graph;
    tyr::UnorderedMap<graphs::VertexIndex, graphs::VertexIndex> m_parent;
    graphs::VertexIndexList m_cycle;

public:
    explicit CycleVisitor(const Graph& graph) : m_graph(graph) {}

    void tree_edge(graphs::EdgeIndex edge) override { m_parent[m_graph.get_target(edge)] = m_graph.get_source(edge); }

    void back_edge(graphs::EdgeIndex edge) override
    {
        if (!m_cycle.empty())
            return;

        const auto source = m_graph.get_source(edge);
        const auto target = m_graph.get_target(edge);

        m_cycle.push_back(target);
        for (auto vertex = source; vertex != target; vertex = m_parent.at(vertex))
            m_cycle.push_back(vertex);
        m_cycle.push_back(target);
        std::ranges::reverse(m_cycle);
    }

    const auto& get_cycle() const noexcept { return m_cycle; }
};

template<typename Graph>
auto find_cycle(const Graph& graph) -> graphs::VertexIndexList
{
    auto visitor = CycleVisitor<Graph>(graph);
    auto sources = graphs::VertexIndexList {};
    sources.reserve(graph.get_num_vertices());
    for (auto vertex : graph.get_vertex_indices())
        sources.push_back(vertex);

    graphs::algorithms::depth_first_visit(graph, sources, visitor);
    return visitor.get_cycle();
}

template<tyr::planning::TaskKind Kind>
ModuleProgramProofVertexLabel<Kind>
make_proof_vertex_label(const EvaluationContext<Kind>& context, bool is_initial, bool is_goal, bool is_alive, bool is_unsolvable)
{
    auto label = ModuleProgramProofVertexLabel<Kind> { context.get_state(),
                                                       context.get_module(),
                                                       context.get_memory_state(),
                                                       context.get_call_stack().size(),
                                                       is_goal ? tyr::float_t(0) : std::numeric_limits<tyr::float_t>::infinity() };

    label.is_initial = is_initial;
    label.is_goal = is_goal;
    label.is_alive = is_alive;
    label.is_unsolvable = is_unsolvable;
    return label;
}

template<tyr::planning::TaskKind Kind>
std::string proof_configuration_key(const EvaluationContext<Kind>& context)
{
    auto os = std::ostringstream {};
    os << tyr::uint_t(context.get_state().get_index()) << '|' << tyr::uint_t(context.get_module().get_index()) << '|'
       << tyr::uint_t(context.get_memory_state().get_index());
    return os.str();
}

template<tyr::planning::TaskKind Kind>
void append_plan_suffix(tyr::planning::LabeledNodeList<Kind>& target, const tyr::planning::SearchResult<Kind>& search_result)
{
    if (!search_result.plan)
        return;

    const auto& suffix = search_result.plan->get_labeled_succ_nodes();
    target.insert(target.end(), suffix.begin(), suffix.end());
}

template<tyr::planning::TaskKind Kind>
void append_single_step_plan(tyr::planning::LabeledNodeList<Kind>& target,
                             const tyr::planning::StateView<Kind>& source_state,
                             const tyr::planning::StateView<Kind>& target_state,
                             const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    if (source_state.get_index() == target_state.get_index())
        return;

    for (const auto& successor : successors)
    {
        if (successor.node.get_state().get_index() == target_state.get_index())
        {
            target.push_back(successor);
            return;
        }
    }
}

inline ModuleProgramProofStatus translate_proof_status(ModuleExecutionStatus status)
{
    switch (status)
    {
        case ModuleExecutionStatus::SUCCESS:
            return ModuleProgramProofStatus::SUCCESS;
        case ModuleExecutionStatus::OUT_OF_TIME:
            return ModuleProgramProofStatus::OUT_OF_TIME;
        case ModuleExecutionStatus::OUT_OF_STATES:
            return ModuleProgramProofStatus::OUT_OF_STATES;
        case detail::ModuleExecutionStatus::FAILURE:
        case detail::ModuleExecutionStatus::NO_APPLICABLE_ACTION:
        case detail::ModuleExecutionStatus::MALFORMED_CALL:
        case ModuleExecutionStatus::SEARCH_FAILURE:
        case ModuleExecutionStatus::LOAD_LIMIT_REACHED:
        case ModuleExecutionStatus::STEP_LIMIT_REACHED:
            return ModuleProgramProofStatus::FAILURE;
    }

    return ModuleProgramProofStatus::FAILURE;
}

}  // namespace detail

template<tyr::planning::TaskKind Kind>
auto prove_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                    ModuleProgramView program,
                    const ModuleProgramSearchOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    auto result = ModuleProgramProofResults<Kind> {};
    result.context = search_context;

    auto dl_builder = runir::kr::dl::semantics::Builder();
    auto dl_denotation_repository_factory = runir::kr::dl::semantics::DenotationRepositoryFactory();
    auto dl_denotation_repository = dl_denotation_repository_factory.create();
    auto modules = std::vector<ModuleView> {};
    for (auto module : program.get_modules())
        modules.push_back(module);

    const auto initial_node = search_context.successor_generator->get_initial_node();
    auto context =
        EvaluationContext<Kind>(initial_node.get_state(), program.get_entry_module(), dl_builder, dl_denotation_repository, {}, {}, {}, {}, std::move(modules));

    auto builder = ModuleProgramProofGraphBuilder<Kind> {};
    auto configuration_to_vertex = std::unordered_map<std::string, graphs::VertexIndex> {};
    auto task_goal_strategy = tyr::planning::ConjunctiveGoalStrategy<Kind>(*search_context.task);
    const auto initial_state = initial_node.get_state();
    const auto static_goal_satisfied = task_goal_strategy.is_static_goal_satisfied(*search_context.task);
    auto is_goal = [&](const tyr::planning::StateView<Kind>& state)
    { return static_goal_satisfied && task_goal_strategy.is_dynamic_goal_satisfied(initial_state, state); };

    auto get_or_create_vertex = [&](const EvaluationContext<Kind>& current, bool is_initial, bool is_alive, bool is_unsolvable)
    {
        auto label = detail::make_proof_vertex_label(current, is_initial, is_goal(current.get_state()), is_alive, is_unsolvable);
        const auto key = detail::proof_configuration_key(current);
        if (const auto it = configuration_to_vertex.find(key); it != configuration_to_vertex.end())
            return std::pair(it->second, false);

        const auto vertex = builder.add_vertex(std::move(label));
        configuration_to_vertex.emplace(key, vertex);
        return std::pair(vertex, true);
    };

    auto current_vertex = get_or_create_vertex(context, true, true, false).first;

    auto add_step_edge = [&](graphs::VertexIndex source, graphs::VertexIndex target, tyr::float_t cost)
    { return builder.add_directed_edge(source, target, ModuleProgramProofEdgeLabel { std::nullopt, cost }); };

    auto finish = [&](ModuleProgramProofStatus status)
    {
        result.status = status;
        auto graph = std::make_shared<ModuleProgramProofGraph<Kind>>(std::move(builder));
        if (result.cycle.empty())
            result.cycle = detail::find_cycle(*graph);
        if (result.status == ModuleProgramProofStatus::SUCCESS && !result.cycle.empty())
            result.status = ModuleProgramProofStatus::FAILURE;
        result.graph = std::move(graph);
        return std::move(result);
    };

    for (std::size_t step = 0; step < options.max_steps; ++step)
    {
        if (is_goal(context.get_state()))
            return finish(ModuleProgramProofStatus::SUCCESS);

        auto load_source = current_vertex;
        for (std::size_t load_step = 0; load_step < options.max_load_steps; ++load_step)
        {
            const auto load_status = execute_next_load(context);
            if (load_status == LoadExecutionStatus::STABLE)
                break;

            const auto terminal = load_status != LoadExecutionStatus::APPLIED;
            const auto [target, created] = get_or_create_vertex(context, false, !terminal, terminal);
            const auto edge = add_step_edge(load_source, target, 0);
            if (!created && target != load_source)
                result.cycle = graphs::VertexIndexList { target, load_source, target };
            current_vertex = target;
            load_source = target;

            if (load_status == LoadExecutionStatus::EMPTY_DENOTATION)
            {
                result.deadend_transitions.push_back(edge);
                result.open_states.push_back(target);
                return finish(ModuleProgramProofStatus::FAILURE);
            }
            if (load_status == LoadExecutionStatus::LOAD_LIMIT_REACHED)
            {
                result.deadend_transitions.push_back(edge);
                return finish(ModuleProgramProofStatus::FAILURE);
            }
            if (is_goal(context.get_state()))
                return finish(ModuleProgramProofStatus::SUCCESS);
        }

        if (current_vertex != load_source)
            current_vertex = load_source;

        const auto source_vertex = current_vertex;
        const auto source_state = context.get_state();
        const auto node = search_context.successor_generator->get_node(context.get_state().get_index());
        const auto successors = search_context.successor_generator->get_labeled_successor_nodes(node);
        const auto rule_status = execute_next_immediate_external_rule(context, successors);

        if (rule_status == RuleExecutionStatus::APPLIED)
        {
            const auto [target, created] = get_or_create_vertex(context, false, true, false);
            add_step_edge(source_vertex, target, context.get_state().get_index() == source_state.get_index() ? tyr::float_t(0) : tyr::float_t(1));
            if (!created && target != source_vertex)
                result.cycle = graphs::VertexIndexList { target, source_vertex, target };
            current_vertex = target;
            continue;
        }

        if (rule_status == RuleExecutionStatus::NO_APPLICABLE_ACTION && context.restore_caller())
        {
            const auto [target, created] = get_or_create_vertex(context, false, true, false);
            add_step_edge(source_vertex, target, 0);
            if (!created && target != source_vertex)
                result.cycle = graphs::VertexIndexList { target, source_vertex, target };
            current_vertex = target;
            continue;
        }

        if (rule_status == RuleExecutionStatus::NO_APPLICABLE_RULE)
        {
            const auto sketch_status = execute_next_sketch_rule(context, successors);
            if (sketch_status == RuleExecutionStatus::APPLIED)
            {
                const auto [target, created] = get_or_create_vertex(context, false, true, false);
                add_step_edge(source_vertex, target, context.get_state().get_index() == source_state.get_index() ? tyr::float_t(0) : tyr::float_t(1));
                if (!created && target != source_vertex)
                    result.cycle = graphs::VertexIndexList { target, source_vertex, target };
                current_vertex = target;
                continue;
            }

            const auto search_result = detail::find_module_program_transition_node(search_context, context, detail::execution_options(options));
            if (search_result.status == tyr::planning::SearchStatus::SOLVED && search_result.goal_node)
            {
                if (const auto rule = find_matching_sketch_rule(context, search_result.goal_node->get_state()))
                {
                    context.set_state(search_result.goal_node->get_state());
                    context.set_memory_state(rule->get_target());
                    const auto [target, created] = get_or_create_vertex(context, false, true, false);
                    add_step_edge(source_vertex, target, search_result.plan ? static_cast<tyr::float_t>(search_result.plan->get_length()) : tyr::float_t(1));
                    if (!created && target != source_vertex)
                        result.cycle = graphs::VertexIndexList { target, source_vertex, target };
                    current_vertex = target;
                    continue;
                }
            }
            else if (search_result.status == tyr::planning::SearchStatus::OUT_OF_TIME)
            {
                return finish(ModuleProgramProofStatus::OUT_OF_TIME);
            }
            else if (search_result.status == tyr::planning::SearchStatus::OUT_OF_STATES || search_result.status == tyr::planning::SearchStatus::OUT_OF_MEMORY)
            {
                return finish(ModuleProgramProofStatus::OUT_OF_STATES);
            }

            if (context.restore_caller())
            {
                const auto [target, created] = get_or_create_vertex(context, false, true, false);
                add_step_edge(source_vertex, target, 0);
                if (!created && target != source_vertex)
                    result.cycle = graphs::VertexIndexList { target, source_vertex, target };
                current_vertex = target;
                continue;
            }
        }

        const auto [target, created] = get_or_create_vertex(context, false, false, true);
        static_cast<void>(created);
        const auto edge = add_step_edge(source_vertex, target, 0);
        result.deadend_transitions.push_back(edge);
        result.open_states.push_back(target);
        return finish(detail::translate_proof_status(rule_status == RuleExecutionStatus::NO_APPLICABLE_ACTION ?
                                                         detail::ModuleExecutionStatus::NO_APPLICABLE_ACTION :
                                                     rule_status == RuleExecutionStatus::MALFORMED_CALL ? detail::ModuleExecutionStatus::MALFORMED_CALL :
                                                                                                          detail::ModuleExecutionStatus::FAILURE));
    }

    const auto [target, created] = get_or_create_vertex(context, false, false, true);
    static_cast<void>(created);
    const auto edge = add_step_edge(current_vertex, target, 0);
    result.deadend_transitions.push_back(edge);
    result.cycle = graphs::VertexIndexList { current_vertex, target, current_vertex };
    return finish(ModuleProgramProofStatus::FAILURE);
}

namespace detail
{

template<tyr::planning::TaskKind Kind>
auto execute_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                      ModuleView entry_module,
                      const std::vector<ModuleView>& modules,
                      const ModuleExecutionOptions<Kind>& options) -> ModuleExecutionResults<Kind>
{
    auto dl_builder = runir::kr::dl::semantics::Builder();
    auto dl_denotation_repository_factory = runir::kr::dl::semantics::DenotationRepositoryFactory();
    auto dl_denotation_repository = dl_denotation_repository_factory.create();
    const auto initial_node = search_context.successor_generator->get_initial_node();
    auto plan_steps = tyr::planning::LabeledNodeList<Kind> {};
    auto context = EvaluationContext<Kind>(initial_node.get_state(), entry_module, dl_builder, dl_denotation_repository, {}, {}, {}, {}, modules);
    auto goal_strategy = tyr::planning::ConjunctiveGoalStrategy<Kind>(*search_context.task);
    const auto initial_state = context.get_state();
    const auto static_goal_satisfied = goal_strategy.is_static_goal_satisfied(*search_context.task);

    auto finish = [&](ModuleExecutionStatus status)
    {
        return ModuleExecutionResults<Kind> {
            status, context.get_state(), context.get_module(), context.get_memory_state(), std::nullopt, 0, context.get_call_stack().size()
        };
    };

    for (std::size_t step = 0; step < options.max_steps; ++step)
    {
        if (static_goal_satisfied && goal_strategy.is_dynamic_goal_satisfied(initial_state, context.get_state()))
        {
            auto result = finish(ModuleExecutionStatus::SUCCESS);
            result.plan = tyr::planning::Plan<Kind>(initial_node, std::move(plan_steps));
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
            auto result = finish(detail::ModuleExecutionStatus::FAILURE);
            result.num_steps = step;
            return result;
        }

        const auto source_state = context.get_state();
        const auto node = search_context.successor_generator->get_node(context.get_state().get_index());
        const auto successors = search_context.successor_generator->get_labeled_successor_nodes(node);
        const auto rule_status = execute_next_immediate_external_rule(context, successors);

        if (rule_status == RuleExecutionStatus::APPLIED)
        {
            detail::append_single_step_plan(plan_steps, source_state, context.get_state(), successors);
            continue;
        }

        if (rule_status == RuleExecutionStatus::NO_APPLICABLE_ACTION && context.restore_caller())
            continue;

        if (rule_status == RuleExecutionStatus::NO_APPLICABLE_RULE)
        {
            const auto sketch_status = execute_next_sketch_rule(context, successors);
            if (sketch_status == RuleExecutionStatus::APPLIED)
            {
                detail::append_single_step_plan(plan_steps, source_state, context.get_state(), successors);
                continue;
            }

            const auto search_result = find_module_program_transition_node(search_context, context, options);
            if (search_result.status == tyr::planning::SearchStatus::SOLVED && search_result.goal_node)
            {
                if (const auto rule = find_matching_sketch_rule(context, search_result.goal_node->get_state()))
                {
                    detail::append_plan_suffix(plan_steps, search_result);
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

        auto status = detail::ModuleExecutionStatus::FAILURE;
        if (rule_status == RuleExecutionStatus::NO_APPLICABLE_ACTION)
            status = detail::ModuleExecutionStatus::NO_APPLICABLE_ACTION;
        else if (rule_status == RuleExecutionStatus::MALFORMED_CALL)
            status = detail::ModuleExecutionStatus::MALFORMED_CALL;

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
                      const ModuleExecutionOptions<Kind>& options) -> ModuleExecutionResults<Kind>
{
    return execute_solution(search_context, entry_module, std::vector<ModuleView> { entry_module }, options);
}

template<tyr::planning::TaskKind Kind>
auto execute_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                      ModuleProgramView program,
                      const ModuleExecutionOptions<Kind>& options) -> ModuleExecutionResults<Kind>
{
    auto modules = std::vector<ModuleView> {};
    for (auto module : program.get_modules())
        modules.push_back(module);
    return execute_solution(search_context, program.get_entry_module(), modules, options);
}

}  // namespace detail

}  // namespace runir::kr::ps::ext

#endif
