#ifndef RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATION_WORKSPACE_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATION_WORKSPACE_HPP_

#include "runir/kr/dl/semantics/interning.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/icp/compatibility.hpp"
#include "runir/kr/ps/icp/detail/execution_step.hpp"
#include "runir/kr/ps/icp/evaluation_environment.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <unordered_map>
#include <vector>

namespace runir::kr::ps::icp::detail
{

/// Shared evaluation and history scratch; individual rule records own no buffers.
template<tyr::TaskKind Kind>
class RuleEvaluationWorkspace
{
    using Concept = runir::kr::dl::ConceptTag;
    using Denotation = runir::kr::dl::semantics::Denotation<Concept>;
    TaskContextPtr<Kind> m_task;
    ProgramView m_program;
    EvaluationEnvironment<Kind> m_environment;
    std::vector<std::vector<std::size_t>> m_reset_predecessors;
    std::vector<bool> m_changed;
    ygg::Index<Denotation> m_empty;

public:
    RuleEvaluationWorkspace(TaskContextPtr<Kind> task, ProgramView program) :
        m_task(task ? std::move(task) : throw std::invalid_argument("ICP requires a task context.")),
        m_program(program),
        m_environment(*m_task, program)
    {
        if (&program.get_context() != m_task->domain_context->icp_repository.get())
            throw std::invalid_argument("ICP requires a program from the domain context repository.");
        const auto planning_task = m_task->search_context->task->get_task();
        auto empty = m_task->dl_builder.template get_builder<Denotation>(static_cast<ygg::uint_t>(planning_task.get_num_objects()));
        m_empty = runir::kr::dl::semantics::insert(*m_task->dl_denotation_repository, *empty, m_task->dl_builder).first.get_index();
        const auto module = program.get_module();
        const auto features = module.template get_features<Concept>();
        m_reset_predecessors.resize(features.size());
        m_changed.reserve(features.size());
        auto slots = std::unordered_map<ygg::uint_t, std::size_t> {};
        for (std::size_t i = 0; i < features.size(); ++i)
            slots.emplace(ygg::uint_t(features[i].get_index()), i);
        for (const auto& pair : module.get_reset_pairs())
            m_reset_predecessors.at(slots.at(ygg::uint_t(pair.after))).push_back(slots.at(ygg::uint_t(pair.before)));
    }

    const auto& get_task_context() const noexcept { return m_task; }
    auto get_program() const noexcept { return m_program; }
    auto& get_environment() noexcept { return m_environment; }
    auto intern(ygg::Data<ProgramState<Kind>>& data) { return insert(*m_task->icp_execution_repository, data).first; }

    ProgramStateView<Kind> initial_state(tyr::planning::StateView<Kind> state)
    {
        const auto module = m_program.get_module();
        auto registers = checkout<runir::kr::dl::semantics::RegisterValues>(m_task->dl_builder);
        registers->concept_values.resize(module.template get_registers<Concept>().size());
        registers->role_values.resize(module.template get_registers<runir::kr::dl::RoleTag>().size());
        const auto stored_registers = insert(*m_task->dl_denotation_repository, *registers).first;
        auto histories = checkout<Histories>(m_task->icp_execution_builder);
        histories->concepts.resize(module.template get_features<Concept>().size(), m_empty);
        auto data = ygg::Data<ProgramState<Kind>> {};
        data.program = m_program.get_index();
        data.state = state.get_index();
        data.memory_state = module.get_entry_memory_state().get_index();
        data.registers = stored_registers.get_index();
        data.histories = insert(*m_task->icp_execution_repository, *histories).first.get_index();
        return intern(data);
    }

    template<runir::kr::ps::dl::TransitionEvaluationContextConcept<IcpFamilyTag, Kind> Transition, StopConcept Stop>
    std::optional<HistoriesView<Kind>>
    update_histories(ProgramStateView<Kind> state, Transition& transition, std::optional<tyr::planning::BorrowedActionBindingView<Kind>> binding, Stop&& stop)
    {
        auto histories = checkout<Histories>(m_task->icp_execution_builder);
        *histories = state.get_histories().get_data();
        const auto previous = state.get_histories().get_concepts();
        const auto features = m_program.get_module().template get_features<Concept>();
        m_changed.assign(features.size(), false);
        bool progress = !binding;
        for (std::size_t i = 0; i < features.size(); ++i)
        {
            if (stop())
                return std::nullopt;
            const auto before = evaluate<Kind>(features[i], transition.get_source_context());
            const auto after = evaluate<Kind>(features[i], transition.get_target_context());
            auto entered = m_task->dl_builder.template get_builder<Denotation>(before.get_data().num_objects);
            entered->get().copy_from(after.get());
            entered->get() -= before.get();
            if (entered->get().intersects(previous[i].get()))
                return std::nullopt;
            m_changed[i] = before.get() != after.get();
            if (entered->get().any())
            {
                if (binding && !progress)
                    for (std::size_t j = 0; j < binding->get_relation().get_original_arity(); ++j)
                        if (entered->get().test(ygg::uint_t(binding->get_objects()[j].get_index())))
                        {
                            progress = true;
                            break;
                        }
                entered->get() |= previous[i].get();
                histories->concepts[i] = runir::kr::dl::semantics::insert(*m_task->dl_denotation_repository, *entered, m_task->dl_builder).first.get_index();
            }
        }
        if (!progress)
            return std::nullopt;
        // Reset after admission and accumulation, including predecessors entering in this step.
        for (std::size_t i = 0; i < m_changed.size(); ++i)
            if (m_changed[i])
                for (const auto predecessor : m_reset_predecessors[i])
                    histories->concepts[predecessor] = m_empty;
        return insert(*m_task->icp_execution_repository, *histories).first;
    }
};

}  // namespace runir::kr::ps::icp::detail

#endif
