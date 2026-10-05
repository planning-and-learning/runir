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

template<runir::kr::FamilyTag Family, typename FeatureTag>
using DlFeature = ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>;

template<runir::kr::FamilyTag Family, typename Categories>
using PsFeatureTypes = ygg::ConcatTypeListsT<ygg::MapTypeListSecondT<Feature, Family, Categories>, ygg::MapTypeListSecondT<DlFeature, Family, Categories>>;

template<runir::kr::FamilyTag Family>
using PsConcreteConditionTypes =
    ygg::TypeList<ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>,
                  ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>,
                  ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>,
                  ConcreteCondition<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>;

template<runir::kr::FamilyTag Family>
using PsConcreteEffectTypes = ygg::TypeList<ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>,
                                            ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>,
                                            ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>,
                                            ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>,
                                            ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>,
                                            ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>;

template<runir::kr::FamilyTag Family>
using PsConditionTypes =
    ygg::ConcatTypeListsT<ygg::TypeList<ConditionVariant<Family>, ConcreteConditionVariant<Family, runir::kr::DlTag>>, PsConcreteConditionTypes<Family>>;

template<runir::kr::FamilyTag Family>
using PsEffectTypes =
    ygg::ConcatTypeListsT<ygg::TypeList<EffectVariant<Family>, ConcreteEffectVariant<Family, runir::kr::DlTag>>, PsConcreteEffectTypes<Family>>;

}  // namespace detail

template<runir::kr::FamilyTag Family>
struct PsFamilyTraits;

template<>
struct PsFamilyTraits<runir::kr::BaseFamilyTag>
{
    using DlFamily = runir::kr::BaseFamilyTag;
    using FeatureCategories = ygg::TypeList<runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::NumericalFeature>;

    using FeatureTypes = detail::PsFeatureTypes<runir::kr::BaseFamilyTag, FeatureCategories>;

    using ConditionTypes = detail::PsConditionTypes<runir::kr::BaseFamilyTag>;

    using EffectTypes = detail::PsEffectTypes<runir::kr::BaseFamilyTag>;
};

template<>
struct PsFamilyTraits<runir::kr::ExtFamilyTag>
{
    using DlFamily = runir::kr::ExtFamilyTag;
    using FeatureCategories = ygg::TypeList<runir::kr::dl::ConceptTag,
                                            runir::kr::dl::RoleTag,
                                            runir::kr::ps::dl::BooleanFeature,
                                            runir::kr::ps::dl::NumericalFeature,
                                            runir::kr::ps::dl::QueryFeature>;

    using FeatureTypes = detail::PsFeatureTypes<runir::kr::ExtFamilyTag, FeatureCategories>;

    using ConditionTypes = detail::PsConditionTypes<runir::kr::ExtFamilyTag>;

    using EffectTypes = detail::PsEffectTypes<runir::kr::ExtFamilyTag>;
};

template<>
struct PsFamilyTraits<runir::kr::IcpFamilyTag>
{
    using DlFamily = runir::kr::ExtFamilyTag;
    using FeatureCategories = PsFamilyTraits<runir::kr::ExtFamilyTag>::FeatureCategories;

    using FeatureTypes = detail::PsFeatureTypes<runir::kr::IcpFamilyTag, FeatureCategories>;

    using ConditionTypes = detail::PsConditionTypes<runir::kr::IcpFamilyTag>;

    using EffectTypes = detail::PsEffectTypes<runir::kr::IcpFamilyTag>;
};

template<>
struct PsFamilyTraits<runir::kr::UnsFamilyTag>
{
    using DlFamily = runir::kr::UnsFamilyTag;
    using FeatureCategories = ygg::TypeList<runir::kr::ps::dl::BooleanFeature>;
    using FeatureTypes = detail::PsFeatureTypes<runir::kr::UnsFamilyTag, FeatureCategories>;
    using ConditionTypes = ygg::TypeList<>;
    using EffectTypes = ygg::TypeList<>;
};

/// DL expression family used by a policy family.
template<runir::kr::FamilyTag Family>
using DlFamilyFor = typename PsFamilyTraits<Family>::DlFamily;

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
