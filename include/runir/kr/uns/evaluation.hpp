#ifndef RUNIR_KR_UNS_EVALUATION_HPP_
#define RUNIR_KR_UNS_EVALUATION_HPP_

#include "runir/kr/uns/dl/evaluation.hpp"
#include "runir/kr/uns/feature_view.hpp"

#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::uns
{

// Evaluate a (wrapper) feature by visiting its concrete language-specific specialization.
template<typename C, typename EvaluationContext>
bool evaluate(ygg::View<ygg::Index<runir::kr::uns::Feature>, C> feature, EvaluationContext& context)
{
    return ygg::visit([&](auto concrete) { return runir::kr::uns::dl::evaluate(concrete, context); }, feature.get_variant());
}

}  // namespace runir::kr::uns

#endif
