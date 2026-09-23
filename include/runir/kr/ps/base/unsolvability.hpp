#ifndef RUNIR_KR_PS_BASE_UNSOLVABILITY_HPP_
#define RUNIR_KR_PS_BASE_UNSOLVABILITY_HPP_

#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/task_context.hpp"
#include "runir/kr/uns/classify.hpp"

namespace runir::kr::ps::base
{

struct NoUnsolvability
{
    template<tyr::TaskKind Kind>
    bool is_unsolvable(const tyr::planning::StateView<Kind>&) const noexcept
    {
        return false;
    }
};

template<tyr::TaskKind Kind>
class ClassifierUnsolvability
{
private:
    runir::kr::TaskContext<Kind>& m_task_context;
    runir::kr::uns::ClassifierView m_classifier;
    runir::kr::dl::semantics::DenotationCaches<runir::kr::UnsFamilyTag> m_caches;

public:
    ClassifierUnsolvability(runir::kr::TaskContext<Kind>& task_context, runir::kr::uns::ClassifierView classifier) :
        m_task_context(task_context),
        m_classifier(classifier)
    {
    }

    bool is_unsolvable(const tyr::planning::StateView<Kind>& state)
    {
        m_caches.clear(false);
        auto context = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::UnsFamilyTag, Kind>(state,
                                                                                                  m_task_context.dl_builder,
                                                                                                  *m_task_context.dl_denotation_repository,
                                                                                                  m_task_context.dl_builder.get_workspace(),
                                                                                                  m_caches);
        return runir::kr::uns::classify(m_classifier, context);
    }
};

}  // namespace runir::kr::ps::base

#endif
