#ifndef RUNIR_KR_PS_FEATURE_DATA_HPP_
#define RUNIR_KR_PS_FEATURE_DATA_HPP_

#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <utility>
#include <variant>

namespace runir::kr::ps
{

/// The concrete feature a feature of Family and FeatureTag holds (one alternative: a DL feature).
template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag>
using FeatureVariant = urd::flat_variant<urd::index<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>>>;

}  // namespace runir::kr::ps

namespace urd
{

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag>
struct data<runir::kr::ps::Feature<Family, FeatureTag>>
{
    urd::index<runir::kr::ps::Feature<Family, FeatureTag>> index;
    runir::kr::ps::FeatureVariant<Family, FeatureTag> variant;

    data() = default;

    explicit data(runir::kr::ps::FeatureVariant<Family, FeatureTag> variant_) : index(), variant(std::move(variant_)) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"variant", &data::variant>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::variant> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::variant> {}; }
};

}  // namespace urd

#endif
