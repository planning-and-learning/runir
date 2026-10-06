#ifndef RUNIR_KR_PS_BASE_EVALUATION_ENVIRONMENT_HPP_
#define RUNIR_KR_PS_BASE_EVALUATION_ENVIRONMENT_HPP_

#include "runir/kr/dl/semantics/evaluation_storage.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/ps/dl/transition_evaluation_context.hpp"
#include "runir/kr/dl/semantics/evaluation_policy.hpp"
#include "runir/kr/task_context.hpp"

#include <utility>
#include <vector>

namespace runir::kr::ps::base
{

template<tyr::TaskKind Kind, runir::kr::dl::semantics::EvaluationPolicyConcept<BaseFamilyTag, Kind> EvaluationPolicy = runir::kr::dl::semantics::DefaultEvaluationPolicy<BaseFamilyTag, Kind>>
class EvaluationEnvironment
{
private:
    using StateDlContext = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::BaseFamilyTag, Kind>;

    runir::kr::dl::semantics::Builder& m_dl_builder;
    runir::kr::dl::semantics::EvaluationStorage<runir::kr::BaseFamilyTag> m_source_storage;
    runir::kr::dl::semantics::EvaluationStorage<runir::kr::BaseFamilyTag> m_target_storage;
    EvaluationPolicy m_policy;

    static auto roots(SketchView sketch)
    {
        auto result = std::vector<runir::kr::dl::semantics::incremental::EvaluationRoot<BaseFamilyTag>> {};
        for (const auto feature : sketch.get_features<runir::kr::ps::dl::BooleanFeature>())
            result.emplace_back(feature.get_expression());
        for (const auto feature : sketch.get_features<runir::kr::ps::dl::NumericalFeature>())
            result.emplace_back(feature.get_expression());
        return result;
    }

public:
    EvaluationEnvironment(runir::kr::TaskContext<Kind>& task_context, SketchView sketch) :
        m_dl_builder(task_context.dl_builder),
        m_source_storage(*task_context.dl_denotation_repository),
        m_target_storage(*task_context.dl_denotation_repository),
        m_policy(*task_context.search_context->task, roots(sketch))
    {
    }

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

    auto make_dl_context(tyr::planning::StateView<Kind> state)
    {
        return m_policy.make_source_context(StateDlContext(std::move(state), m_dl_builder, m_source_storage));
    }

    auto make_dl_transition_context(tyr::planning::StateView<Kind> source_state, tyr::planning::StateView<Kind> target_state)
    {
        return make_evaluation_transition_context<BaseFamilyTag>(
            make_dl_context(std::move(source_state)),
            m_policy.make_target_context(StateDlContext(std::move(target_state), m_dl_builder, m_target_storage)));
    }
};

}  // namespace runir::kr::ps::base

#endif
