#ifndef RUNIR_KR_PS_EXT_EVALUATION_HPP_
#define RUNIR_KR_PS_EXT_EVALUATION_HPP_

#include "runir/kr/ps/ext/feature_view.hpp"

#include <tyr/common/types.hpp>
#include <tyr/common/variant.hpp>

namespace runir::kr::ps::ext
{

template<typename FeatureTag, typename C, typename EvaluationContext>
auto evaluate(tyr::View<tyr::Index<runir::kr::ps::ext::Feature<FeatureTag>>, C> feature, EvaluationContext& context)
{
    return tyr::visit([&](auto child) { return evaluate(child, context); }, feature.get_variant());
}

}  // namespace runir::kr::ps::ext

#endif
