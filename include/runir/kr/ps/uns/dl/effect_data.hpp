#ifndef RUNIR_KR_PS_UNS_DL_EFFECT_DATA_HPP_
#define RUNIR_KR_PS_UNS_DL_EFFECT_DATA_HPP_

#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/effect_index.hpp"
#include "runir/kr/ps/feature_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ConcreteEffectVariant<runir::kr::UnsFamilyTag, runir::kr::DlTag>>
{
    using Variant = ::cista::offset::variant<
        Index<runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>,
        Index<runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>,
        Index<runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>>,
        Index<runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>>,
        Index<runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>>,
        Index<runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>>;

    Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::UnsFamilyTag, runir::kr::DlTag>> index;
    Variant value;

    Data() = default;
    Data(Variant value_) : index(), value(std::move(value_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(value);
    }

    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<typename FeatureTag, typename ObservationTag>
struct Data<runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>
{
    Index<runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>> index;
    Index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, FeatureTag>> feature;

    Data() = default;
    Data(Index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, FeatureTag>> feature_) : index(), feature(feature_) {}

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
