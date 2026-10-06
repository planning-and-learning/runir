#ifndef RUNIR_KR_PS_EVALUATION_HPP_
#define RUNIR_KR_PS_EVALUATION_HPP_

#include "runir/kr/ps/family_traits.hpp"
#include "runir/kr/dl/semantics/evaluation_context.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps
{

template<tyr::TaskKind Kind,
         runir::kr::FamilyTag Family,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         typename C,
         runir::kr::dl::semantics::EvaluationContextConcept<DlFamilyFor<Family>, Kind> Context>
auto evaluate(ygg::View<ygg::Index<Feature<Family, FeatureTag>>, C> feature, Context& context)
{
    return ygg::visit([&](auto child) { return evaluate<Kind>(child, context); }, feature.get_variant());
}

}  // namespace runir::kr::ps

#endif
