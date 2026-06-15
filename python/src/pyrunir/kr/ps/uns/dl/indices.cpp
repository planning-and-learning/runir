#include "module.hpp"

#include <runir/kr/ps/uns/repository.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::uns::dl
{

void bind_indices(nb::module_& m)
{
    ygg::bind_index<ygg::Index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::BooleanFeature>>>(m, "BooleanFeatureIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::NumericalFeature>>>(m, "NumericalFeatureIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>>(
        m,
        "ConcreteBooleanFeatureIndex");
    ygg::bind_index<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>>(
        m,
        "ConcreteNumericalFeatureIndex");

    ygg::bind_index<ygg::Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::UnsFamilyTag, runir::kr::DlTag>>>(m, "ConcreteConditionVariantIndex");
    ygg::bind_index<ygg::Index<
        runir::kr::ps::ConcreteCondition<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>>(
        m,
        "PositiveBooleanConditionIndex");
    ygg::bind_index<ygg::Index<
        runir::kr::ps::ConcreteCondition<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>>(
        m,
        "NegativeBooleanConditionIndex");
    ygg::bind_index<ygg::Index<
        runir::kr::ps::ConcreteCondition<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>>>(
        m,
        "EqualZeroNumericalConditionIndex");
    ygg::bind_index<ygg::Index<
        runir::kr::ps::ConcreteCondition<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>>(
        m,
        "GreaterZeroNumericalConditionIndex");

    ygg::bind_index<ygg::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::UnsFamilyTag, runir::kr::DlTag>>>(m, "ConcreteEffectVariantIndex");
    ygg::bind_index<
        ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>>(
        m,
        "PositiveBooleanEffectIndex");
    ygg::bind_index<
        ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>>(
        m,
        "NegativeBooleanEffectIndex");
    ygg::bind_index<
        ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>>>(
        m,
        "UnchangedBooleanEffectIndex");
    ygg::bind_index<ygg::Index<
        runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>>>(
        m,
        "IncreasesNumericalEffectIndex");
    ygg::bind_index<ygg::Index<
        runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>>>(
        m,
        "DecreasesNumericalEffectIndex");
    ygg::bind_index<ygg::Index<
        runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>>(
        m,
        "UnchangedNumericalEffectIndex");
}

}  // namespace runir::kr::ps::uns::dl
