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

/// Owns borrowed input views. Each demand updates only its dependency graph;
/// results already read in this state remain stable as other roots are evaluated.
template<FamilyTag Family, tyr::TaskKind Kind, StateEvaluationContextConcept<Family, Kind> Context>
struct DeltaEvaluationContext
{
    incremental::EvaluationGraph<Family, Kind>& graph;
    Context inputs;
    const incremental::Delta<Family>& delta;
};

template<tyr::TaskKind Kind, FamilyTag Family, runir::kr::dl::CategoryTag Category, StateEvaluationContextConcept<Family, Kind> Context>
auto evaluate(runir::kr::dl::FamilyConstructorView<Family, Category> expression, DeltaEvaluationContext<Family, Kind, Context>& context)
{
    return context.graph.evaluate(context.graph.get_index(expression), context.inputs, context.delta);
}

template<tyr::TaskKind Kind, FamilyTag Family, StateEvaluationContextConcept<Family, Kind> Context>
auto evaluate(runir::kr::dl::FamilyQueryView<Family> expression, DeltaEvaluationContext<Family, Kind, Context>& context)
{
    return ygg::make_view(context.graph.evaluate(context.graph.get_index(expression), context.inputs, context.delta),
                          *context.inputs.get_state().get_task().get_repository());
}

/// Independent source and target graphs retain their input snapshots and result buffers.
/// Differences are computed between evaluations, including backward search moves;
/// invocation changes explicitly invalidate dynamic baselines.
template<FamilyTag Family, tyr::TaskKind Kind>
class DeltaEvaluationPolicy
{
    class Evaluation
    {
        const tyr::planning::Task<Kind>& m_task;
        incremental::EvaluationGraph<Family, Kind> m_graph;
        ygg::Builder<tyr::planning::State<Kind>> m_state;
        ygg::Data<RegisterValues> m_registers;
        incremental::Delta<Family> m_delta;
        bool m_initialized = false;
        bool m_ready = false;

    public:
        Evaluation(const tyr::planning::Task<Kind>& task, std::span<const incremental::EvaluationRoot<Family>> roots) : m_task(task), m_graph(task, roots) {}
        void reset() noexcept { m_ready = false; }
        void invalidate() noexcept { m_initialized = m_ready = false; }

        template<StateEvaluationContextConcept<Family, Kind> Context>
        auto prepare(Context& context)
        {
            const auto& task = m_task;
            if (&context.get_state().get_task() != &task)
                throw std::invalid_argument("Delta evaluation requires states from its planning task.");
            const auto& repository = *task.get_repository();
            if (!m_ready || !m_graph.is_valid())
            {
                const auto initialized = std::exchange(m_initialized, false) && m_graph.is_valid();
                if (initialized)
                {
                    if constexpr (std::same_as<Family, ExtFamilyTag>)
                        m_delta.template assign<Kind>(ygg::make_view(m_state, task),
                                                      ygg::make_view(m_registers, repository),
                                                      context.get_state(),
                                                      context.registers());
                    else
                        m_delta.template assign<Kind>(ygg::make_view(m_state, task), context.get_state());
                    m_graph.advance();
                }
                else
                    m_graph.reset();
                m_state = context.get_state().get_state_builder();
                if constexpr (std::same_as<Family, ExtFamilyTag>)
                    assign(m_registers, context.registers());
                m_initialized = m_ready = true;
            }
            return DeltaEvaluationContext<Family, Kind, Context> { m_graph, context, m_delta };
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
