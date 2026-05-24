#ifndef RUNIR_KR_PS_EXT_DL_EVALUATION_HPP_
#define RUNIR_KR_PS_EXT_DL_EVALUATION_HPP_

#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/ps/ext/dl/feature_view.hpp"
#include "runir/kr/ps/ext/evaluation.hpp"

#include <tyr/common/types.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>

namespace runir::kr::ps::ext
{

template<typename FeatureTag, typename C, tyr::planning::TaskKind Kind>
auto evaluate(tyr::View<tyr::Index<ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C> feature,
              runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context)
{
    return runir::kr::dl::semantics::evaluate(feature.get_feature(), context).get();
}

}  // namespace runir::kr::ps::ext

#endif
