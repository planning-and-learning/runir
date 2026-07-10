#ifndef RUNIR_KR_PS_FAMILY_TRAITS_HPP_
#define RUNIR_KR_PS_FAMILY_TRAITS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <yggdrasil/core/type_list.hpp>

namespace runir::kr::ps
{

template<runir::kr::FamilyTag Family>
struct PsFamilyTraits;

template<>
struct PsFamilyTraits<runir::kr::BaseFamilyTag>
{
    using DlFamily = runir::kr::BaseFamilyTag;
    using FeatureCategories = ygg::TypeList<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::NumericalFeature>;
    using ConditionLanguages = ygg::TypeList<runir::kr::DlTag>;
    using EffectLanguages = ygg::TypeList<runir::kr::DlTag>;

    using FeatureTypes = ygg::TypeList<Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>,
                                       Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>,
                                       ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>,
                                       ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>;

    using ConditionTypes =
        ygg::TypeList<ConditionVariant<runir::kr::BaseFamilyTag>,
                      ConcreteConditionVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>,
                      ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>,
                      ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>,
                      ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>,
                      ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>;

    using EffectTypes =
        ygg::TypeList<EffectVariant<runir::kr::BaseFamilyTag>,
                      ConcreteEffectVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>,
                      ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>,
                      ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>,
                      ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>,
                      ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>,
                      ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>,
                      ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>;
};

template<>
struct PsFamilyTraits<runir::kr::ExtFamilyTag>
{
    using DlFamily = runir::kr::ExtFamilyTag;
    using FeatureCategories =
        ygg::TypeList<runir::kr::dl::ConceptTag, runir::kr::dl::RoleTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::NumericalFeature>;
    using ConditionLanguages = ygg::TypeList<runir::kr::DlTag>;
    using EffectLanguages = ygg::TypeList<runir::kr::DlTag>;

    using FeatureTypes = ygg::TypeList<Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>,
                                       Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>,
                                       Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>,
                                       Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>,
                                       ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::ConceptTag>,
                                       ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::RoleTag>,
                                       ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>,
                                       ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>;

    using ConditionTypes =
        ygg::TypeList<ConditionVariant<runir::kr::ExtFamilyTag>,
                      ConcreteConditionVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>,
                      ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>,
                      ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>,
                      ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>,
                      ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>;

    using EffectTypes =
        ygg::TypeList<EffectVariant<runir::kr::ExtFamilyTag>,
                      ConcreteEffectVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>,
                      ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>,
                      ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>,
                      ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>,
                      ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>,
                      ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>,
                      ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>;
};

template<runir::kr::FamilyTag Family>
using PsFeatureTypes = typename PsFamilyTraits<Family>::FeatureTypes;

template<runir::kr::FamilyTag Family>
using PsConditionTypes = typename PsFamilyTraits<Family>::ConditionTypes;

template<runir::kr::FamilyTag Family>
using PsEffectTypes = typename PsFamilyTraits<Family>::EffectTypes;

template<runir::kr::FamilyTag Family>
using PsCoreTypes = ygg::ConcatTypeListsT<PsFeatureTypes<Family>, PsConditionTypes<Family>, PsEffectTypes<Family>>;

}  // namespace runir::kr::ps

#endif
