#ifndef RUNIR_KR_PS_EVALUATION_HPP_
#define RUNIR_KR_PS_EVALUATION_HPP_

#include "runir/kr/dl/semantics/evaluation_context.hpp"
#include "runir/kr/ps/family_traits.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <urd/elements/flat.hpp>

namespace runir::kr::ps
{

template<tyr::TaskKind Kind,
         runir::kr::FamilyTag Family,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         typename C,
         runir::kr::dl::semantics::EvaluationContextConcept<Kind> Context>
auto evaluate(urd::view<Feature<Family, FeatureTag>, C> feature, Context& context)
{
    return std::visit([&](auto child) { return evaluate<Kind>(child, context); }, feature.get_variant());
}

}  // namespace runir::kr::ps

#endif
