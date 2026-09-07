#ifndef RUNIR_KR_PS_EXT_DL_CONDITION_DATA_HPP_
#define RUNIR_KR_PS_EXT_DL_CONDITION_DATA_HPP_

#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/condition_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>
{
    using Variant = ::cista::offset::variant<
        Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>,
        Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>,
        Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>>,
        Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>>;

    Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>> index;
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

template<typename FeatureTag, typename ObservationTag>
struct Data<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>
{
    Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>> index;
    Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>> feature;

    Data() = default;
    Data(Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>> feature_) : index(), feature(feature_) {}

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
