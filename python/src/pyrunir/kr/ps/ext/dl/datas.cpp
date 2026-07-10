#include "pyrunir/kr/ps/ext/dl/module.hpp"

#include <runir/kr/ps/ext/datas.hpp>
#include <runir/kr/ps/ext/dl/feature_data.hpp>
#include <string>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext::dl
{
namespace
{

template<typename T>
void bind_data(nb::module_& m, const std::string& name)
{
    using Data = ygg::Data<T>;

    auto cls = nb::class_<Data>(m, name.c_str()).def(nb::init<>());

    if constexpr (requires { &Data::index; })
        cls.def_rw("index", &Data::index);
    if constexpr (requires { &Data::value; })
        cls.def_rw("value", &Data::value);
    if constexpr (requires { &Data::feature; })
        cls.def_rw("feature", &Data::feature);
    if constexpr (requires { &Data::symbol; })
        cls.def_rw("symbol", &Data::symbol);
}

}  // namespace

void bind_datas(nb::module_& m)
{
    bind_data<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>(m, "ConceptFeatureData");
    bind_data<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>(m, "RoleFeatureData");
    bind_data<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>(m, "BooleanFeatureData");
    bind_data<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>(m, "NumericalFeatureData");
    bind_data<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::ConceptTag>>(m, "ConcreteConceptFeatureData");
    bind_data<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::RoleTag>>(m, "ConcreteRoleFeatureData");
    bind_data<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>(m, "ConcreteBooleanFeatureData");
    bind_data<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>(m,
                                                                                                                              "ConcreteNumericalFeatureData");

    bind_data<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>(m, "ConcreteConditionVariantData");
    bind_data<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>(
        m,
        "PositiveBooleanConditionData");
    bind_data<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>(
        m,
        "NegativeBooleanConditionData");
    bind_data<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>>(
        m,
        "EqualZeroNumericalConditionData");
    bind_data<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>(
        m,
        "GreaterZeroNumericalConditionData");

    bind_data<runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>(m, "ConcreteEffectVariantData");
    bind_data<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>(
        m,
        "PositiveBooleanEffectData");
    bind_data<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>(
        m,
        "NegativeBooleanEffectData");
    bind_data<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>>(
        m,
        "UnchangedBooleanEffectData");
    bind_data<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>>(
        m,
        "IncreasesNumericalEffectData");
    bind_data<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>>(
        m,
        "DecreasesNumericalEffectData");
    bind_data<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>(
        m,
        "UnchangedNumericalEffectData");
}

}  // namespace runir::kr::ps::ext::dl
