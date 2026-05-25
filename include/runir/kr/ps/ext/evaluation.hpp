#ifndef RUNIR_KR_PS_EXT_EVALUATION_HPP_
#define RUNIR_KR_PS_EXT_EVALUATION_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/ps/ext/dl/feature_view.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
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

template<runir::kr::dl::CategoryTag Category, typename C, tyr::planning::TaskKind Kind>
auto evaluate(tyr::View<tyr::Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, Category>>, C> constructor,
              runir::kr::ps::ext::EvaluationContext<Kind>& context)
{
    return context.with_current_dl_context([&](auto& dl_context) { return runir::kr::dl::semantics::evaluate(constructor, dl_context); });
}

template<typename FeatureTag, typename C, tyr::planning::TaskKind Kind>
auto evaluate(tyr::View<tyr::Index<ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C> feature, runir::kr::ps::ext::EvaluationContext<Kind>& context)
{
    return runir::kr::ps::ext::evaluate(feature.get_feature(), context).get();
}

template<runir::kr::dl::CategoryTag Category, tyr::planning::TaskKind Kind>
auto evaluate_argument(tyr::Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, Category>> argument,
                       runir::kr::ps::ext::EvaluationContext<Kind>& context)
{
    return runir::kr::ps::ext::evaluate(tyr::make_view(argument, context.get_dl_repository()), context);
}

}  // namespace runir::kr::ps::ext

#endif
