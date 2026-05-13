#include "module.hpp"

#include <runir/kr/gp/repository.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::gp
{

namespace gpd = runir::kr::gp::dl;

void bind_indices(nb::module_& m)
{
    tyr::bind_index<tyr::Index<Feature<gpd::BooleanFeature>>>(m, "BooleanFeatureIndex");
    tyr::bind_index<tyr::Index<Feature<gpd::NumericalFeature>>>(m, "NumericalFeatureIndex");
    tyr::bind_index<tyr::Index<ConcreteFeature<runir::kr::DlTag, gpd::BooleanFeature>>>(m, "ConcreteBooleanFeatureIndex");
    tyr::bind_index<tyr::Index<ConcreteFeature<runir::kr::DlTag, gpd::NumericalFeature>>>(m, "ConcreteNumericalFeatureIndex");

    tyr::bind_index<tyr::Index<ConditionVariant>>(m, "ConditionVariantIndex");
    tyr::bind_index<tyr::Index<ConcreteConditionVariant<runir::kr::DlTag>>>(m, "ConcreteConditionVariantIndex");
    tyr::bind_index<tyr::Index<ConcreteCondition<runir::kr::DlTag, gpd::BooleanFeature, gpd::Positive>>>(m, "PositiveBooleanConditionIndex");
    tyr::bind_index<tyr::Index<ConcreteCondition<runir::kr::DlTag, gpd::BooleanFeature, gpd::Negative>>>(m, "NegativeBooleanConditionIndex");
    tyr::bind_index<tyr::Index<ConcreteCondition<runir::kr::DlTag, gpd::NumericalFeature, gpd::EqualZero>>>(m, "EqualZeroNumericalConditionIndex");
    tyr::bind_index<tyr::Index<ConcreteCondition<runir::kr::DlTag, gpd::NumericalFeature, gpd::GreaterZero>>>(m, "GreaterZeroNumericalConditionIndex");

    tyr::bind_index<tyr::Index<EffectVariant>>(m, "EffectVariantIndex");
    tyr::bind_index<tyr::Index<ConcreteEffectVariant<runir::kr::DlTag>>>(m, "ConcreteEffectVariantIndex");
    tyr::bind_index<tyr::Index<ConcreteEffect<runir::kr::DlTag, gpd::BooleanFeature, gpd::Positive>>>(m, "PositiveBooleanEffectIndex");
    tyr::bind_index<tyr::Index<ConcreteEffect<runir::kr::DlTag, gpd::BooleanFeature, gpd::Negative>>>(m, "NegativeBooleanEffectIndex");
    tyr::bind_index<tyr::Index<ConcreteEffect<runir::kr::DlTag, gpd::BooleanFeature, gpd::Unchanged>>>(m, "UnchangedBooleanEffectIndex");
    tyr::bind_index<tyr::Index<ConcreteEffect<runir::kr::DlTag, gpd::NumericalFeature, gpd::Increases>>>(m, "IncreasesNumericalEffectIndex");
    tyr::bind_index<tyr::Index<ConcreteEffect<runir::kr::DlTag, gpd::NumericalFeature, gpd::Decreases>>>(m, "DecreasesNumericalEffectIndex");
    tyr::bind_index<tyr::Index<ConcreteEffect<runir::kr::DlTag, gpd::NumericalFeature, gpd::Unchanged>>>(m, "UnchangedNumericalEffectIndex");

    tyr::bind_index<tyr::Index<Rule>>(m, "RuleIndex");
    tyr::bind_index<tyr::Index<Policy>>(m, "PolicyIndex");
}

}  // namespace runir::kr::gp
