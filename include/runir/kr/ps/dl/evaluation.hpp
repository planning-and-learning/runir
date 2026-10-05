#ifndef RUNIR_KR_PS_DL_EVALUATION_HPP_
#define RUNIR_KR_PS_DL_EVALUATION_HPP_

#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/evaluation.hpp"
#include "runir/kr/ps/family_traits.hpp"

namespace runir::kr::ps
{

template<tyr::TaskKind Kind,
         runir::kr::FamilyTag Family,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         typename C,
         runir::kr::dl::semantics::StateEvaluationContextConcept<DlFamilyFor<Family>, Kind> Context>
auto evaluate(ygg::View<ygg::Index<ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>>, C> feature, Context& context)
{
    return runir::kr::dl::semantics::evaluate<Kind>(feature.get_feature(), context);
}

}  // namespace runir::kr::ps

#endif
