#ifndef RUNIR_KR_PS_EVALUATION_HPP_
#define RUNIR_KR_PS_EVALUATION_HPP_

#include "runir/kr/ps/feature_view.hpp"

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>

namespace runir::kr::ps
{

template<runir::kr::FamilyTag Family, typename FeatureTag, typename C, typename EvaluationContext>
auto evaluate(ygg::View<ygg::Index<Feature<Family, FeatureTag>>, C> feature, EvaluationContext& context)
{
    return ygg::visit([&](auto child) { return evaluate(child, context); }, feature.get_variant());
}

template<runir::kr::FamilyTag Family, typename FeatureTag, typename C, tyr::planning::TaskKind Kind, typename EvaluationContext>
auto evaluate(ygg::View<ygg::Index<Feature<Family, FeatureTag>>, C> feature, tyr::planning::StateView<Kind> state, EvaluationContext& context)
{
    return ygg::visit([&](auto child) { return evaluate(child, state, context); }, feature.get_variant());
}

}  // namespace runir::kr::ps

#endif
