#ifndef RUNIR_KR_PS_DL_EFFECT_BUILDER_HPP_
#define RUNIR_KR_PS_DL_EFFECT_BUILDER_HPP_

#include "runir/kr/ps/dl/effect_data.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of ConcreteEffect: its record's members, each in its built form (urd::built_t).
template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, runir::kr::ps::dl::EffectObservationTag<FeatureTag> ObservationTag, typename C>
struct builder<runir::kr::ps::ConcreteEffect<Family, FeatureTag, ObservationTag>, C>
{
    urd::key<runir::kr::ps::ConcreteEffect<Family, FeatureTag, ObservationTag>> index;
    urd::built_t<urd::index<runir::kr::ps::Feature<Family, FeatureTag>>, C> feature;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"feature", &builder::feature>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::feature> {}; }
};

}  // namespace urd

#endif
