#ifndef RUNIR_KR_PS_UNSOLVABILITY_HPP_
#define RUNIR_KR_PS_UNSOLVABILITY_HPP_

#include "runir/kr/dl/semantics/evaluation_storage.hpp"
#include "runir/kr/dl/semantics/evaluation_policy.hpp"
#include "runir/kr/task_context.hpp"
#include "runir/kr/uns/classify.hpp"

#include <vector>

namespace runir::kr::ps
{

template<tyr::TaskKind Kind>
struct NoUnsolvability
{
    template<tyr::planning::StateViewConcept<Kind> State>
    bool is_unsolvable(const State&) const noexcept
    {
        return false;
    }
};

template<tyr::TaskKind Kind, runir::kr::dl::semantics::EvaluationPolicyConcept<runir::kr::UnsFamilyTag, Kind> Policy = runir::kr::dl::semantics::DefaultEvaluationPolicy<runir::kr::UnsFamilyTag, Kind>>
class ClassifierUnsolvability
{
private:
    runir::kr::TaskContext<Kind>& m_task_context;
    runir::kr::uns::ClassifierView m_classifier;
    runir::kr::dl::semantics::EvaluationStorage<runir::kr::UnsFamilyTag> m_storage;
    Policy m_policy;

public:
    ClassifierUnsolvability(runir::kr::TaskContext<Kind>& task_context, runir::kr::uns::ClassifierView classifier) :
        m_task_context(task_context),
        m_classifier(classifier),
        m_storage(*task_context.dl_denotation_repository),
        m_policy(*task_context.search_context->task,
                 [&]
                 {
                     auto roots = std::vector<runir::kr::dl::semantics::incremental::EvaluationRoot<runir::kr::UnsFamilyTag>> {};
                     for (const auto feature : classifier.get_features())
                         roots.emplace_back(feature.get_expression());
                     return roots;
                 }())
    {
    }

    template<tyr::planning::StateViewConcept<Kind> State>
    bool is_unsolvable(const State& state)
    {
        m_storage.reset_dynamic();
        m_policy.reset_source();
        auto context = m_policy.make_source_context(
            runir::kr::dl::semantics::StateEvaluationContext<runir::kr::UnsFamilyTag, Kind, State>(state, m_task_context.dl_builder, m_storage));
        return runir::kr::uns::classify<Kind>(m_classifier, context);
    }
};

}  // namespace runir::kr::ps

#endif
