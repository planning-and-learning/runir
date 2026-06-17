#ifndef RUNIR_KR_UNS_DL_EVALUATION_HPP_
#define RUNIR_KR_UNS_DL_EVALUATION_HPP_

#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/uns/evaluation_context.hpp"
#include "runir/kr/uns/dl/feature_view.hpp"

#include <tyr/planning/declarations.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::uns::dl
{

template<typename C, tyr::planning::TaskKind Kind>
bool evaluate(ygg::View<ygg::Index<runir::kr::uns::dl::Feature>, C> feature,
              runir::kr::dl::semantics::EvaluationContext<runir::kr::UnsFamilyTag, Kind>& context)
{
    return runir::kr::dl::semantics::evaluate(feature.get_feature(), context).get();
}

}  // namespace runir::kr::uns::dl

#endif
