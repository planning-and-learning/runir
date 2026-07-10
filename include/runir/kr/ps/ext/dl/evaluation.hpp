#ifndef RUNIR_KR_PS_EXT_DL_EVALUATION_HPP_
#define RUNIR_KR_PS_EXT_DL_EVALUATION_HPP_

#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/ps/ext/dl/feature_view.hpp"
#include "runir/kr/ps/ext/evaluation.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::ext
{

template<typename FeatureTag, typename C, tyr::planning::TaskKind Kind>
auto evaluate(ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>, C> feature,
              runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context)
{
    return runir::kr::dl::semantics::evaluate(feature.get_feature(), context).get();
}

template<typename FeatureTag, typename C, tyr::planning::TaskKind Kind>
auto evaluate(ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>, C> feature,
              runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context,
              runir::kr::dl::semantics::EvaluationWorkspace& workspace)
{
    return runir::kr::dl::semantics::evaluate(feature.get_feature(), context, workspace).get();
}

template<typename FeatureTag, typename C, tyr::planning::TaskKind Kind>
auto evaluate(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> feature,
              runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::ext::evaluate(child, context); }, feature.get_variant());
}

template<typename FeatureTag, typename C, tyr::planning::TaskKind Kind>
auto evaluate(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> feature,
              runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, Kind>& context,
              runir::kr::dl::semantics::EvaluationWorkspace& workspace)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::ext::evaluate(child, context, workspace); }, feature.get_variant());
}

}  // namespace runir::kr::ps::ext

#endif
