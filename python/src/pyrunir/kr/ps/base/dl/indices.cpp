#include "module.hpp"

#include <runir/kr/ps/base/repository.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::ps::base::dl
{

void bind_indices(nb::module_& m)
{
    tyr::bind_index<tyr::Index<runir::kr::ps::base::Feature<BooleanFeature>>>(m, "BooleanFeatureIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::base::Feature<NumericalFeature>>>(m, "NumericalFeatureIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::base::ConcreteFeature<runir::kr::DlTag, BooleanFeature>>>(m, "ConcreteBooleanFeatureIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::base::ConcreteFeature<runir::kr::DlTag, NumericalFeature>>>(m, "ConcreteNumericalFeatureIndex");

    tyr::bind_index<tyr::Index<runir::kr::ps::base::ConcreteConditionVariant<runir::kr::DlTag>>>(m, "ConcreteConditionVariantIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::base::ConcreteCondition<runir::kr::DlTag, BooleanFeature, Positive>>>(m, "PositiveBooleanConditionIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::base::ConcreteCondition<runir::kr::DlTag, BooleanFeature, Negative>>>(m, "NegativeBooleanConditionIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::base::ConcreteCondition<runir::kr::DlTag, NumericalFeature, EqualZero>>>(m, "EqualZeroNumericalConditionIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::base::ConcreteCondition<runir::kr::DlTag, NumericalFeature, GreaterZero>>>(m,
                                                                                                                         "GreaterZeroNumericalConditionIndex");

    tyr::bind_index<tyr::Index<runir::kr::ps::base::ConcreteEffectVariant<runir::kr::DlTag>>>(m, "ConcreteEffectVariantIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::base::ConcreteEffect<runir::kr::DlTag, BooleanFeature, Positive>>>(m, "PositiveBooleanEffectIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::base::ConcreteEffect<runir::kr::DlTag, BooleanFeature, Negative>>>(m, "NegativeBooleanEffectIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::base::ConcreteEffect<runir::kr::DlTag, BooleanFeature, Unchanged>>>(m, "UnchangedBooleanEffectIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::base::ConcreteEffect<runir::kr::DlTag, NumericalFeature, Increases>>>(m, "IncreasesNumericalEffectIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::base::ConcreteEffect<runir::kr::DlTag, NumericalFeature, Decreases>>>(m, "DecreasesNumericalEffectIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::base::ConcreteEffect<runir::kr::DlTag, NumericalFeature, Unchanged>>>(m, "UnchangedNumericalEffectIndex");
}

}  // namespace runir::kr::ps::base::dl
