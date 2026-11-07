#ifndef RUNIR_KR_PS_FAMILY_TRAITS_HPP_
#define RUNIR_KR_PS_FAMILY_TRAITS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <cista/containers/vector.h>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>

namespace runir::kr::ps
{

namespace detail
{

template<runir::kr::FamilyTag Family, typename FeatureTag>
using DlFeature = ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>;

template<runir::kr::FamilyTag Family, typename Categories>
using PsFeatureTypes = boost::mp11::mp_append<boost::mp11::mp_transform_q<boost::mp11::mp_bind_front<Feature, Family>, Categories>, boost::mp11::mp_transform_q<boost::mp11::mp_bind_front<DlFeature, Family>, Categories>>;

template<runir::kr::FamilyTag Family>
using PsConcreteConditionTypes = boost::mp11::mp_list<ConcreteCondition<Family, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>,
                                               ConcreteCondition<Family, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>,
                                               ConcreteCondition<Family, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::EqualZero>,
                                               ConcreteCondition<Family, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::GreaterZero>>;

template<runir::kr::FamilyTag Family>
using PsConcreteEffectTypes = boost::mp11::mp_list<ConcreteEffect<Family, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>,
                                            ConcreteEffect<Family, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>,
                                            ConcreteEffect<Family, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Unchanged>,
                                            ConcreteEffect<Family, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Increases>,
                                            ConcreteEffect<Family, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Decreases>,
                                            ConcreteEffect<Family, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Unchanged>>;

}  // namespace detail

/// A condition or effect: one of the family's concrete records. Stored as it is, not interned.
template<runir::kr::FamilyTag Family>
using ConditionVariant = urd::index_variant<detail::PsConcreteConditionTypes<Family>>;
template<runir::kr::FamilyTag Family>
using ConditionList = urd::flat_vector<ConditionVariant<Family>>;
template<runir::kr::FamilyTag Family>
using EffectVariant = urd::index_variant<detail::PsConcreteEffectTypes<Family>>;
template<runir::kr::FamilyTag Family>
using EffectList = urd::flat_vector<EffectVariant<Family>>;

template<runir::kr::FamilyTag Family>
struct PsFamilyTraits;

template<>
struct PsFamilyTraits<runir::kr::BaseFamilyTag>
{
    using FeatureCategories = boost::mp11::mp_list<runir::kr::dl::BooleanTag, runir::kr::dl::NumericalTag>;

    using FeatureTypes = detail::PsFeatureTypes<runir::kr::BaseFamilyTag, FeatureCategories>;

    using ConditionTypes = detail::PsConcreteConditionTypes<runir::kr::BaseFamilyTag>;

    using EffectTypes = detail::PsConcreteEffectTypes<runir::kr::BaseFamilyTag>;
};

template<>
struct PsFamilyTraits<runir::kr::ExtFamilyTag>
{
    using FeatureCategories =
        boost::mp11::mp_list<runir::kr::dl::ConceptTag, runir::kr::dl::RoleTag, runir::kr::dl::BooleanTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::QueryFeature>;

    using FeatureTypes = detail::PsFeatureTypes<runir::kr::ExtFamilyTag, FeatureCategories>;

    using ConditionTypes = detail::PsConcreteConditionTypes<runir::kr::ExtFamilyTag>;

    using EffectTypes = detail::PsConcreteEffectTypes<runir::kr::ExtFamilyTag>;
};

template<>
struct PsFamilyTraits<runir::kr::IcpFamilyTag>
{
    using FeatureCategories = PsFamilyTraits<runir::kr::ExtFamilyTag>::FeatureCategories;

    using FeatureTypes = detail::PsFeatureTypes<runir::kr::IcpFamilyTag, FeatureCategories>;

    using ConditionTypes = detail::PsConcreteConditionTypes<runir::kr::IcpFamilyTag>;

    using EffectTypes = detail::PsConcreteEffectTypes<runir::kr::IcpFamilyTag>;
};

template<>
struct PsFamilyTraits<runir::kr::UnsFamilyTag>
{
    using FeatureCategories = boost::mp11::mp_list<runir::kr::dl::BooleanTag>;
    using FeatureTypes = detail::PsFeatureTypes<runir::kr::UnsFamilyTag, FeatureCategories>;
    using ConditionTypes = boost::mp11::mp_list<>;
    using EffectTypes = boost::mp11::mp_list<>;
};

template<runir::kr::FamilyTag Family>
using PsFeatureTypes = typename PsFamilyTraits<Family>::FeatureTypes;

template<runir::kr::FamilyTag Family>
using PsConditionTypes = typename PsFamilyTraits<Family>::ConditionTypes;

template<runir::kr::FamilyTag Family>
using PsEffectTypes = typename PsFamilyTraits<Family>::EffectTypes;

template<runir::kr::FamilyTag Family>
using PsCoreTypes = boost::mp11::mp_append<PsFeatureTypes<Family>, PsConditionTypes<Family>, PsEffectTypes<Family>>;

}  // namespace runir::kr::ps

#endif
