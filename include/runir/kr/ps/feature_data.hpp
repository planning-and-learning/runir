#ifndef RUNIR_KR_PS_FEATURE_DATA_HPP_
#define RUNIR_KR_PS_FEATURE_DATA_HPP_

#include "runir/kr/ps/feature_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family, typename FeatureTag>
struct Data<runir::kr::ps::Feature<Family, FeatureTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>>>;

    Index<runir::kr::ps::Feature<Family, FeatureTag>> index;
    Variant variant;

    Data() = default;
    Data(Variant variant_) : index(), variant(std::move(variant_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(variant);
    }

    auto cista_members() const noexcept { return std::tie(index, variant); }
    auto identifying_members() const noexcept { return std::tie(variant); }
};

}  // namespace ygg

#endif
