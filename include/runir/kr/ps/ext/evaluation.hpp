#ifndef RUNIR_KR_PS_EXT_EVALUATION_HPP_
#define RUNIR_KR_PS_EXT_EVALUATION_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/ps/ext/dl/feature_view.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::ext
{

template<typename FeatureTag, typename C, tyr::planning::TaskKind Kind>
auto evaluate(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> feature,
              EvaluationContext<Kind>& context,
              EvaluationEnvironment<Kind>& environment)
{
    return ygg::visit([&](auto child) { return evaluate(child, context, environment); }, feature.get_variant());
}

template<runir::kr::dl::CategoryTag Category, typename C, tyr::planning::TaskKind Kind>
auto evaluate(ygg::View<ygg::Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, Category>>, C> constructor,
              EvaluationContext<Kind>& context,
              EvaluationEnvironment<Kind>& environment)
{
    auto dl_context = environment.make_dl_context(context);
    return runir::kr::dl::semantics::evaluate(constructor, dl_context, environment.get_dl_workspace());
}

template<typename FeatureTag, typename C, tyr::planning::TaskKind Kind>
auto evaluate(ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>, C> feature,
              EvaluationContext<Kind>& context,
              EvaluationEnvironment<Kind>& environment)
{
    return runir::kr::ps::ext::evaluate(feature.get_feature(), context, environment).get();
}

template<runir::kr::dl::CategoryTag Category, tyr::planning::TaskKind Kind>
auto evaluate_argument(ygg::Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, Category>> argument,
                       EvaluationContext<Kind>& context,
                       EvaluationEnvironment<Kind>& environment)
{
    return runir::kr::ps::ext::evaluate(ygg::make_view(argument, environment.get_dl_repository()), context, environment);
}

}  // namespace runir::kr::ps::ext

#endif
