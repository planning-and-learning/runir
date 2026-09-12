#ifndef RUNIR_KR_PS_FAMILY_TRAITS_HPP_
#define RUNIR_KR_PS_FAMILY_TRAITS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <yggdrasil/core/type_list.hpp>

namespace runir::kr::ps
{

namespace detail
{

template<runir::kr::FamilyTag Family>
using PsConditionTypes = ygg::TypeList<ConditionVariant<Family>,
                                       ConcreteConditionVariant<Family, runir::kr::DlTag>,
                                       ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>,
                                       ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>,
                                       ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>,
                                       ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>;

template<runir::kr::FamilyTag Family>
using PsEffectTypes = ygg::TypeList<EffectVariant<Family>,
                                    ConcreteEffectVariant<Family, runir::kr::DlTag>,
                                    ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>,
                                    ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>,
                                    ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>,
                                    ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>,
                                    ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>,
                                    ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>;

}  // namespace detail

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

    using ConditionTypes = detail::PsConditionTypes<runir::kr::BaseFamilyTag>;

    using EffectTypes = detail::PsEffectTypes<runir::kr::BaseFamilyTag>;
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

    using ConditionTypes = detail::PsConditionTypes<runir::kr::ExtFamilyTag>;

    using EffectTypes = detail::PsEffectTypes<runir::kr::ExtFamilyTag>;
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
