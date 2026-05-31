#ifndef RUNIR_KR_PS_BASE_DL_EVALUATION_HPP_
#define RUNIR_KR_PS_BASE_DL_EVALUATION_HPP_

#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/ps/base/dl/feature_view.hpp"
#include "runir/kr/ps/evaluation.hpp"

#include <yggdrasil/core/types.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>

namespace runir::kr::ps
{

template<typename FeatureTag, typename C, tyr::planning::TaskKind Kind>
auto evaluate(ygg::View<ygg::Index<ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C> feature,
              runir::kr::dl::semantics::EvaluationContext<runir::kr::BaseFamilyTag, Kind>& context)
{
    return runir::kr::dl::semantics::evaluate(feature.get_feature(), context).get();
}

template<typename FeatureTag, typename C, tyr::planning::TaskKind Kind, typename EvaluationContext>
auto evaluate(ygg::View<ygg::Index<ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C> feature,
              tyr::planning::StateView<Kind>,
              EvaluationContext& context)
{
    return evaluate(feature, context);
}

}  // namespace runir::kr::ps

#endif
