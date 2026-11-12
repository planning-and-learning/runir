#ifndef RUNIR_KR_PS_DL_EFFECT_DATA_HPP_
#define RUNIR_KR_PS_DL_EFFECT_DATA_HPP_

#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <utility>
#include <variant>

namespace urd
{

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, runir::kr::ps::dl::EffectObservationTag<FeatureTag> ObservationTag>
struct data<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>
{
    urd::index<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>> index;
    urd::index<runir::kr::ps::Feature<Family, FeatureTag>> feature;

    data() = default;
    data(urd::index<runir::kr::ps::Feature<Family, FeatureTag>> feature_) : index(), feature(std::move(feature_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::feature> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::feature> {}; }
};

}  // namespace urd

#endif
