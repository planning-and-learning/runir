#include "module.hpp"

#include <runir/kr/ps/base/repository.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::ps::base::dl
{

void bind_indices(nb::module_& m)
{
    tyr::bind_index<tyr::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>>(m, "BooleanFeatureIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>>(m, "NumericalFeatureIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>>(
        m,
        "ConcreteBooleanFeatureIndex");
    tyr::bind_index<tyr::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>>(
        m,
        "ConcreteNumericalFeatureIndex");

    tyr::bind_index<tyr::Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>>(m, "ConcreteConditionVariantIndex");
    tyr::bind_index<tyr::Index<
        runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>>(
        m,
        "PositiveBooleanConditionIndex");
    tyr::bind_index<tyr::Index<
        runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>>(
        m,
        "NegativeBooleanConditionIndex");
    tyr::bind_index<tyr::Index<
        runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>>>(
        m,
        "EqualZeroNumericalConditionIndex");
    tyr::bind_index<tyr::Index<
        runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>>(
        m,
        "GreaterZeroNumericalConditionIndex");

    tyr::bind_index<tyr::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>>(m, "ConcreteEffectVariantIndex");
    tyr::bind_index<
        tyr::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>>(
        m,
        "PositiveBooleanEffectIndex");
    tyr::bind_index<
        tyr::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>>(
        m,
        "NegativeBooleanEffectIndex");
    tyr::bind_index<
        tyr::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>>>(
        m,
        "UnchangedBooleanEffectIndex");
    tyr::bind_index<tyr::Index<
        runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>>>(
        m,
        "IncreasesNumericalEffectIndex");
    tyr::bind_index<tyr::Index<
        runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>>>(
        m,
        "DecreasesNumericalEffectIndex");
    tyr::bind_index<tyr::Index<
        runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>>(
        m,
        "UnchangedNumericalEffectIndex");
}

}  // namespace runir::kr::ps::base::dl
