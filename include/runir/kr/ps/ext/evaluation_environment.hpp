#ifndef RUNIR_KR_PS_EXT_EVALUATION_ENVIRONMENT_HPP_
#define RUNIR_KR_PS_EXT_EVALUATION_ENVIRONMENT_HPP_

#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation_storage.hpp"
#include "runir/kr/dl/semantics/ext/state_evaluation_context.hpp"
#include "runir/kr/ps/dl/transition_evaluation_context.hpp"
#include "runir/kr/dl/semantics/evaluation_policy.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind, runir::kr::dl::semantics::EvaluationPolicyConcept<ExtFamilyTag, Kind> EvaluationPolicy = runir::kr::dl::semantics::DefaultEvaluationPolicy<ExtFamilyTag, Kind>>
class EvaluationEnvironment
{
private:
    struct ModuleEvaluation
    {
        ModuleView module_;
        EvaluationPolicy policy;

        ModuleEvaluation(const tyr::planning::Task<Kind>& task,
                         ModuleView module,
                         std::span<const runir::kr::dl::semantics::incremental::EvaluationRoot<ExtFamilyTag>> roots) :
            module_(module),
            policy(task, roots)
        {
        }
    };
    struct Invocation
    {
        ModuleView module_;
        runir::kr::dl::semantics::CallArgumentsView arguments;
        std::optional<CallStackView<Kind>> caller;

        bool operator==(const Invocation&) const = default;
    };

    runir::kr::dl::semantics::Builder& m_dl_builder;
    runir::kr::dl::semantics::DenotationRepository& m_dl_denotation_repository;
    ExecutionRepository<Kind>& m_execution_repository;
    runir::kr::dl::semantics::EvaluationStorage<runir::kr::ExtFamilyTag> m_dl_storage;
    runir::kr::dl::semantics::EvaluationStorage<runir::kr::ExtFamilyTag> m_dl_target_storage;
    runir::kr::dl::semantics::DenotationCaches<runir::kr::ExtFamilyTag> m_call_caches;
    ProgramView m_program;
    std::vector<ModuleEvaluation> m_modules;
    ModuleEvaluation* m_current = nullptr;
    std::optional<Invocation> m_invocation;

    auto& current_policy()
    {
        if (!m_current)
            throw std::logic_error("Feature evaluation requires a selected source invocation.");
        return m_current->policy;
    }

public:
    EvaluationEnvironment(runir::kr::TaskContext<Kind>& task_context, ProgramView program) :
        m_dl_builder(task_context.dl_builder),
        m_dl_denotation_repository(*task_context.dl_denotation_repository),
        m_execution_repository(*task_context.execution_repository),
        m_dl_storage(m_dl_denotation_repository),
        m_dl_target_storage(m_dl_denotation_repository),
        m_program(program)
    {
        m_modules.reserve(program.get_modules().size());
        auto roots = std::vector<runir::kr::dl::semantics::incremental::EvaluationRoot<ExtFamilyTag>> {};
        for (const auto module_ : program.get_modules())
        {
            roots.clear();
            const auto append = [&]<typename FeatureTag>()
            {
                for (const auto feature : module_.template get_features<FeatureTag>())
                    roots.emplace_back(feature.get_expression());
            };
            append.template operator()<runir::kr::dl::ConceptTag>();
            append.template operator()<runir::kr::dl::RoleTag>();
            append.template operator()<runir::kr::ps::dl::BooleanFeature>();
            append.template operator()<runir::kr::ps::dl::NumericalFeature>();
            append.template operator()<runir::kr::ps::dl::QueryFeature>();
            m_modules.emplace_back(*task_context.search_context->task, module_, roots);
        }
    }
    EvaluationEnvironment(const EvaluationEnvironment&) = delete;
    EvaluationEnvironment& operator=(const EvaluationEnvironment&) = delete;
    EvaluationEnvironment(EvaluationEnvironment&&) = default;

    auto& get_dl_storage() noexcept { return m_dl_storage; }
    auto& get_dl_target_storage() noexcept { return m_dl_target_storage; }
    auto& get_dl_caches() noexcept { return m_dl_storage.get_caches(); }
    auto& get_dl_target_caches() noexcept { return m_dl_target_storage.get_caches(); }

    void reset_source() noexcept
    {
        m_call_caches.reset_dynamic();
        m_dl_storage.reset_dynamic();
        if (m_current)
            m_current->policy.reset_source();
    }

    void reset_target() noexcept
    {
        m_dl_target_storage.reset_dynamic();
        if (m_current)
            m_current->policy.reset_target();
    }

    template<ProgramStateViewConcept<Kind> S>
    void select_source(S state)
    {
        const auto module_ = state.get_module_state();
        const auto invocation = Invocation { module_.get_module(), module_.get_arguments(), state.get_call_stack() };
        if (m_invocation == invocation)
            return;
        const auto found = std::ranges::find(m_modules, invocation.module_, &ModuleEvaluation::module_);
        if (found == m_modules.end())
            throw std::invalid_argument("Feature evaluation requires a module from the prepared program.");
        m_current = &*found;
        m_current->policy.invalidate();
        m_invocation = invocation;
    }

    /// Contexts borrow stored register and argument data. Cache invalidation remains explicit.
    template<ProgramStateViewConcept<Kind> S>
    auto make_dl_context(S state)
    {
        const auto program = state.get_program();
        if (&state.get_context() != &m_execution_repository || &program.get_context() != &m_program.get_context() || program != m_program)
            throw std::invalid_argument("EvaluationEnvironment requires an execution state from the selected task and program.");
        select_source(state);
        const auto module_state = state.get_module_state();
        return make_dl_context(module_state.get_state(), module_state.get_arguments(), module_state.get_registers());
    }

    template<tyr::planning::StateViewConcept<Kind> S, runir::kr::dl::semantics::RegisterValuesViewConcept R>
    auto make_dl_context(S state, runir::kr::dl::semantics::CallArgumentsView arguments, R registers)
    {
        using StateDlContext = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::ExtFamilyTag, Kind, S, R>;
        auto context = StateDlContext(std::move(state), m_dl_builder, m_dl_storage, arguments, registers);
        return current_policy().make_source_context(std::move(context));
    }

    /// Call argument roots enter task storage directly; their intermediates use
    /// the ordinary source storage. Argument inputs already belong to this task.
    template<tyr::planning::StateViewConcept<Kind> S, runir::kr::dl::semantics::RegisterValuesViewConcept R>
    auto make_call_context(S state, runir::kr::dl::semantics::CallArgumentsView arguments, R registers)
    {
        if (&arguments.get_context() != &m_dl_denotation_repository)
            throw std::invalid_argument("Call argument evaluation requires arguments from the selected task repository.");
        using StateDlContext = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::ExtFamilyTag, Kind, S, R>;
        return StateDlContext(std::move(state), m_dl_builder, m_call_caches, m_dl_denotation_repository, m_dl_storage, arguments, registers);
    }

    template<tyr::planning::StateViewConcept<Kind> S, runir::kr::dl::semantics::RegisterValuesViewConcept R>
    auto
    make_dl_transition_context(S source_state, S target_state, runir::kr::dl::semantics::CallArgumentsView arguments, R source_registers, R target_registers)
    {
        using StateDlContext = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::ExtFamilyTag, Kind, S, R>;
        auto source = StateDlContext(std::move(source_state), m_dl_builder, m_dl_storage, arguments, source_registers);
        auto target = StateDlContext(std::move(target_state), m_dl_builder, m_dl_target_storage, arguments, target_registers);
        return make_evaluation_transition_context<ExtFamilyTag>(current_policy().make_source_context(std::move(source)),
                                                                current_policy().make_target_context(std::move(target)));
    }
};

}  // namespace runir::kr::ps::ext

#endif
