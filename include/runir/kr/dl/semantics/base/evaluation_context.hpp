#ifndef RUNIR_KR_DL_SEMANTICS_BASE_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_DL_SEMANTICS_BASE_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/evaluation_context.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::dl::semantics
{

template<tyr::planning::TaskKind Kind>
class EvaluationContext<runir::kr::BaseFamilyTag, Kind> : public BaseEvaluationContext<EvaluationContext<runir::kr::BaseFamilyTag, Kind>, Kind>
{
private:
    using Base = BaseEvaluationContext<EvaluationContext<runir::kr::BaseFamilyTag, Kind>, Kind>;

public:
    EvaluationContext(tyr::planning::StateView<Kind> state, Builder& builder, DenotationRepository& denotation_repository) noexcept :
        Base(std::move(state), builder, denotation_repository)
    {
    }
};

}  // namespace runir::kr::dl::semantics

#endif
