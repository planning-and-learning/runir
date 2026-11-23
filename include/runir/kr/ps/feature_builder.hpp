#ifndef RUNIR_KR_PS_FEATURE_BUILDER_HPP_
#define RUNIR_KR_PS_FEATURE_BUILDER_HPP_

#include "runir/kr/ps/feature_data.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of Feature: its record's members, each in its built form (urd::built_t).
template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
struct builder<runir::kr::ps::Feature<Family, FeatureTag>, C>
{
    urd::key<runir::kr::ps::Feature<Family, FeatureTag>> index;
    urd::built_t<runir::kr::ps::FeatureVariant<Family, FeatureTag>, C> variant;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"variant", &builder::variant>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::variant> {}; }
};

}  // namespace urd

#endif
