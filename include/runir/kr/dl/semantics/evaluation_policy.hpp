#ifndef RUNIR_KR_DL_SEMANTICS_EVALUATION_POLICY_HPP_
#define RUNIR_KR_DL_SEMANTICS_EVALUATION_POLICY_HPP_

#include "runir/kr/dl/semantics/evaluation_context.hpp"
#include "runir/kr/dl/semantics/incremental/evaluation.hpp"

#include <concepts>
#include <span>
#include <stdexcept>
#include <utility>

namespace runir::kr::dl::semantics
{

/// Preserve the ordinary memoized, interned evaluation contexts.
template<FamilyTag Family, tyr::TaskKind Kind>
class FullEvaluationPolicy
{
public:
    FullEvaluationPolicy(const tyr::planning::Task<Kind>&, std::span<const incremental::EvaluationRoot<Family>>) noexcept {}
    void reset_source() noexcept {}
    void reset_target() noexcept {}
    void invalidate() noexcept {}
    template<StateEvaluationContextConcept<Family, Kind> Context>
    auto make_source_context(Context context)
    {
        return context;
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    auto make_target_context(Context context)
    {
        return context;
    }
};

/// Evaluates in the graph of a DeltaEvaluationPolicy: first demand in a state brings a node up to date; results
/// already read in this state remain stable as other roots are evaluated.
template<tyr::TaskKind Kind, FamilyTag Family, runir::kr::dl::CategoryTag Category, StateEvaluationContextConcept<Family, Kind> Context>
auto evaluate(runir::kr::dl::FamilyConstructorView<Family, Category> expression, incremental::StateEvaluation<Family, Kind, Context>& context)
{
    return context.evaluate(context.graph().get_index(expression));
}

template<tyr::TaskKind Kind, FamilyTag Family, StateEvaluationContextConcept<Family, Kind> Context>
auto evaluate(runir::kr::dl::FamilyQueryView<Family> expression, incremental::StateEvaluation<Family, Kind, Context>& context)
{
    return ygg::make_view(context.evaluate(context.graph().get_index(expression)), *context.context().get_state().get_task().get_repository());
}

/// Independent source and target evaluations retain their input snapshots and result buffers.
/// Differences are computed between evaluations, including backward search moves;
/// invocation changes explicitly invalidate dynamic baselines.
template<FamilyTag Family, tyr::TaskKind Kind>
class DeltaEvaluationPolicy
{
    class Evaluation
    {
        const tyr::planning::Task<Kind>* m_task;
        incremental::IncrementalEvaluation<Family, Kind> m_evaluation;
        ygg::Builder<tyr::planning::State<Kind>> m_state;
        ygg::Data<RegisterValues> m_registers;
        incremental::Delta<Family> m_delta;
        bool m_has_previous = false;  ///< m_state holds the previously entered state.
        bool m_entered = false;       ///< The current state is entered.

    public:
        Evaluation(const tyr::planning::Task<Kind>& task, std::span<const incremental::EvaluationRoot<Family>> roots) : m_task(&task), m_evaluation(task, roots) {}
        void reset() noexcept { m_entered = false; }
        void invalidate() noexcept { m_has_previous = m_entered = false; }

        template<StateEvaluationContextConcept<Family, Kind> Context>
        incremental::StateEvaluation<Family, Kind, Context> prepare(Context& context)
        {
            const auto& task = *m_task;
            if (&context.get_state().get_task() != &task)
                throw std::invalid_argument("Delta evaluation requires states from its planning task.");
            if (m_entered)
                return m_evaluation.resume(context, m_delta, context.get_workspace().get_database_workspace());
            auto evaluation = [&]
            {
                if (!std::exchange(m_has_previous, false))
                    return m_evaluation.enter(context);
                if constexpr (std::same_as<Family, ExtFamilyTag>)
                    m_delta.template assign<Kind>(ygg::make_view(m_state, task),
                                                  ygg::make_view(m_registers, *task.get_repository()),
                                                  context.get_state(),
                                                  context.registers());
                else
                    m_delta.template assign<Kind>(ygg::make_view(m_state, task), context.get_state());
                return m_evaluation.enter(context, m_delta, context.get_workspace().get_database_workspace());
            }();
            m_state = context.get_state().get_state_builder();
            if constexpr (std::same_as<Family, ExtFamilyTag>)
                assign(m_registers, context.registers());
            m_has_previous = m_entered = true;
            return evaluation;
        }
    };

    Evaluation m_source;
    Evaluation m_target;

public:
    DeltaEvaluationPolicy(const tyr::planning::Task<Kind>& task, std::span<const incremental::EvaluationRoot<Family>> roots) :
        m_source(task, roots),
        m_target(task, roots)
    {
    }
    DeltaEvaluationPolicy(const DeltaEvaluationPolicy&) = delete;
    DeltaEvaluationPolicy& operator=(const DeltaEvaluationPolicy&) = delete;
    DeltaEvaluationPolicy(DeltaEvaluationPolicy&&) = default;
    DeltaEvaluationPolicy& operator=(DeltaEvaluationPolicy&&) = default;

    void reset_source() noexcept { m_source.reset(); }
    void reset_target() noexcept { m_target.reset(); }
    void invalidate() noexcept
    {
        m_source.invalidate();
        m_target.invalidate();
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    auto make_source_context(Context context)
    {
        return m_source.prepare(context);
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    auto make_target_context(Context context)
    {
        return m_target.prepare(context);
    }
};

/// Provide source and target feature contexts while retaining reusable evaluation storage.
template<typename Policy, typename Family, typename Kind>
concept EvaluationPolicyConcept = FamilyTag<Family> && tyr::TaskKind<Kind>
                                  && std::constructible_from<Policy, const tyr::planning::Task<Kind>&, std::span<const incremental::EvaluationRoot<Family>>>
                                  && requires(Policy& policy, StateEvaluationContext<Family, Kind> context) {
                                         { policy.reset_source() } noexcept -> std::same_as<void>;
                                         { policy.reset_target() } noexcept -> std::same_as<void>;
                                         { policy.invalidate() } noexcept -> std::same_as<void>;
                                         { policy.make_source_context(context) } -> EvaluationContextConcept<Family, Kind>;
                                         { policy.make_target_context(context) } -> EvaluationContextConcept<Family, Kind>;
                                     };

template<FamilyTag Family, tyr::TaskKind Kind>
#if RUNIR_DELTA_EVALUATION
using DefaultEvaluationPolicy = DeltaEvaluationPolicy<Family, Kind>;
#else
using DefaultEvaluationPolicy = FullEvaluationPolicy<Family, Kind>;
#endif

}  // namespace runir::kr::dl::semantics

#endif
