#ifndef RUNIR_KR_PS_DL_CONDITION_DATA_HPP_
#define RUNIR_KR_PS_DL_CONDITION_DATA_HPP_

#include "runir/kr/ps/condition_index.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/family_traits.hpp"
#include "runir/kr/ps/feature_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family>
struct Data<runir::kr::ps::ConcreteConditionVariant<Family, runir::kr::DlTag>>
{
    using Variant = ygg::ApplyTypeListT<::cista::offset::variant, ygg::MapTypeListT<Index, runir::kr::ps::detail::PsConcreteConditionTypes<Family>>>;

    Index<runir::kr::ps::ConcreteConditionVariant<Family, runir::kr::DlTag>> index;
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

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, runir::kr::ps::dl::ConditionObservationTag<FeatureTag> ObservationTag>
struct Data<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>
{
    Index<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>> index;
    Index<runir::kr::ps::Feature<Family, FeatureTag>> feature;

    Data() = default;
    Data(Index<runir::kr::ps::Feature<Family, FeatureTag>> feature_) : index(), feature(feature_) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(feature);
    }

    auto cista_members() const noexcept { return std::tie(index, feature); }
    auto identifying_members() const noexcept { return std::tie(feature); }
};

}  // namespace ygg

#endif
