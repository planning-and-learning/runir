#ifndef RUNIR_KR_PS_ICP_EVALUATION_ENVIRONMENT_HPP_
#define RUNIR_KR_PS_ICP_EVALUATION_ENVIRONMENT_HPP_

#include "runir/kr/dl/semantics/evaluation_storage.hpp"
#include "runir/kr/dl/semantics/ext/state_evaluation_context.hpp"
#include "runir/kr/ps/dl/transition_evaluation_context.hpp"
#include "runir/kr/dl/semantics/evaluation_policy.hpp"
#include "runir/kr/ps/icp/execution_view.hpp"
#include "runir/kr/task_context.hpp"

#include <vector>

namespace runir::kr::ps::icp
{

template<tyr::TaskKind Kind, runir::kr::dl::semantics::EvaluationPolicyConcept<ExtFamilyTag, Kind> EvaluationPolicy = runir::kr::dl::semantics::DefaultEvaluationPolicy<ExtFamilyTag, Kind>>
class EvaluationEnvironment
{
    using StateContext = runir::kr::dl::semantics::StateEvaluationContext<ExtFamilyTag, Kind>;
    TaskContext<Kind>& m_task;
    ProgramView m_program;
    runir::kr::dl::semantics::EvaluationStorage<ExtFamilyTag> m_source_storage, m_target_storage;
    runir::kr::dl::semantics::CallArgumentsView m_arguments;
    EvaluationPolicy m_policy;

    static auto roots(ProgramView program)
    {
        auto result = std::vector<runir::kr::dl::semantics::incremental::EvaluationRoot<ExtFamilyTag>> {};
        const auto append = [&]<typename FeatureTag>()
        {
            for (const auto feature : program.get_module().template get_features<FeatureTag>())
                result.emplace_back(feature.get_expression());
        };
        append.template operator()<runir::kr::dl::ConceptTag>();
        append.template operator()<runir::kr::dl::RoleTag>();
        append.template operator()<runir::kr::dl::BooleanTag>();
        append.template operator()<runir::kr::dl::NumericalTag>();
        append.template operator()<runir::kr::ps::dl::QueryFeature>();
        return result;
    }

    static auto empty_arguments(TaskContext<Kind>& task)
    {
        auto data = checkout<runir::kr::dl::semantics::CallArguments>(task.dl_builder);
        return insert(*task.dl_denotation_repository, *data).first;
    }

public:
    EvaluationEnvironment(TaskContext<Kind>& task, ProgramView program) :
        m_task(task),
        m_program(program),
        m_source_storage(*task.dl_denotation_repository),
        m_target_storage(*task.dl_denotation_repository),
        m_arguments(empty_arguments(task)),
        m_policy(*task.search_context->task, roots(program))
    {
    }

    auto get_program() const noexcept { return m_program; }
    auto& get_dl_repository() noexcept { return m_program.get_context().get_dl_repository(); }
    auto& get_dl_caches() noexcept { return m_source_storage.get_caches(); }
    auto& get_dl_target_caches() noexcept { return m_target_storage.get_caches(); }
    void reset_source() noexcept
    {
        m_source_storage.reset_dynamic();
        m_policy.reset_source();
    }
    void reset_target() noexcept
    {
        m_target_storage.reset_dynamic();
        m_policy.reset_target();
    }

    /// Contexts borrow stored data; callers clear dynamic caches before evaluating a new configuration.
    auto make_dl_context(ProgramStateView<Kind> state)
    {
        if (&state.get_context() != m_task.icp_execution_repository.get() || state.get_program() != m_program)
            throw std::invalid_argument("ICP evaluation requires a state from the selected task and program.");
        return make_dl_context(state.get_state(), state.get_registers());
    }
    auto make_dl_context(tyr::planning::StateView<Kind> state, runir::kr::dl::semantics::RegisterValuesView registers)
    {
        return m_policy.make_source_context(StateContext(state, m_task.dl_builder, m_source_storage, m_arguments, registers));
    }
    auto make_dl_transition_context(tyr::planning::StateView<Kind> source,
                                    tyr::planning::StateView<Kind> target,
                                    runir::kr::dl::semantics::RegisterValuesView source_registers,
                                    runir::kr::dl::semantics::RegisterValuesView target_registers)
    {
        return make_evaluation_transition_context<IcpFamilyTag>(
            make_dl_context(source, source_registers),
            m_policy.make_target_context(StateContext(target, m_task.dl_builder, m_target_storage, m_arguments, target_registers)));
    }
};

}

#endif
