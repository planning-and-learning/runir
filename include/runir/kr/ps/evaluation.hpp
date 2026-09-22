#ifndef RUNIR_KR_PS_EVALUATION_HPP_
#define RUNIR_KR_PS_EVALUATION_HPP_

#include "runir/kr/dl/semantics/state_evaluation_context.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps
{

template<runir::kr::FamilyTag Family, typename FeatureTag, typename C, tyr::TaskKind Kind>
auto evaluate(ygg::View<ygg::Index<Feature<Family, FeatureTag>>, C> feature, runir::kr::dl::semantics::StateEvaluationContext<Family, Kind>& context)
{
    return ygg::visit([&](auto child) { return evaluate(child, context); }, feature.get_variant());
}

}  // namespace runir::kr::ps

#endif
