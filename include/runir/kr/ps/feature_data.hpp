#ifndef RUNIR_KR_PS_FEATURE_DATA_HPP_
#define RUNIR_KR_PS_FEATURE_DATA_HPP_

#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <variant>
#include <urd/elements/flat.hpp>

namespace urd
{

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag>
struct data<runir::kr::ps::Feature<Family, FeatureTag>>
{
    using Variant = ::cista::offset::variant<urd::index<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>>>;

    urd::index<runir::kr::ps::Feature<Family, FeatureTag>> index;
    Variant variant;

    data() = default;
    data(Variant variant_) : index(), variant(std::move(variant_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::variant> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::variant> {}; }
};

}  // namespace urd

#endif
