#ifndef RUNIR_KR_PS_DL_EFFECT_DATA_HPP_
#define RUNIR_KR_PS_DL_EFFECT_DATA_HPP_

#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/effect_index.hpp"
#include "runir/kr/ps/feature_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family>
struct Data<runir::kr::ps::ConcreteEffectVariant<Family, runir::kr::DlTag>>
{
    using Variant = ::cista::offset::variant<
        Index<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>,
        Index<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>,
        Index<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>>,
        Index<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>>,
        Index<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>>,
        Index<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>>;

    Index<runir::kr::ps::ConcreteEffectVariant<Family, runir::kr::DlTag>> index;
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

template<runir::kr::FamilyTag Family, typename FeatureTag, typename ObservationTag>
struct Data<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>
{
    Index<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>> index;
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
