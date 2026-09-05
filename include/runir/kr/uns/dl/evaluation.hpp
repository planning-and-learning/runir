#ifndef RUNIR_KR_UNS_DL_EVALUATION_HPP_
#define RUNIR_KR_UNS_DL_EVALUATION_HPP_

#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/uns/evaluation_context.hpp"
#include "runir/kr/ps/evaluation.hpp"
#include "runir/kr/uns/dl/feature_view.hpp"

#include <tyr/planning/declarations.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps
{

template<typename C, tyr::TaskKind Kind>
bool evaluate(ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>, C> feature,
              runir::kr::dl::semantics::EvaluationContext<runir::kr::UnsFamilyTag, Kind>& context)
{
    return runir::kr::dl::semantics::evaluate(feature.get_feature(), context).get();
}

}  // namespace runir::kr::ps

#endif
