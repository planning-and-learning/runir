#ifndef RUNIR_KR_GP_EFFECT_DATA_HPP_
#define RUNIR_KR_GP_EFFECT_DATA_HPP_

#include "runir/kr/gp/dl/declarations.hpp"
#include "runir/kr/gp/dl/effect_data.hpp"
#include "runir/kr/gp/effect_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::gp::EffectVariantTag>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::gp::EffectTag<runir::kr::gp::dl::BooleanFeatureTag, runir::kr::gp::dl::BecomesTrueTag>>,
                                             Index<runir::kr::gp::EffectTag<runir::kr::gp::dl::BooleanFeatureTag, runir::kr::gp::dl::BecomesFalseTag>>,
                                             Index<runir::kr::gp::EffectTag<runir::kr::gp::dl::BooleanFeatureTag, runir::kr::gp::dl::UnchangedTag>>,
                                             Index<runir::kr::gp::EffectTag<runir::kr::gp::dl::NumericalFeatureTag, runir::kr::gp::dl::IncreasesTag>>,
                                             Index<runir::kr::gp::EffectTag<runir::kr::gp::dl::NumericalFeatureTag, runir::kr::gp::dl::DecreasesTag>>,
                                             Index<runir::kr::gp::EffectTag<runir::kr::gp::dl::NumericalFeatureTag, runir::kr::gp::dl::UnchangedTag>>>;

    Index<runir::kr::gp::EffectVariantTag> index;
    Variant value;

    Data() = default;
    Data(Variant value_) : index(), value(std::move(value_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(value);
    }

    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

template<typename FeatureTypeTag, typename ChangeTag>
struct Data<runir::kr::gp::EffectTag<FeatureTypeTag, ChangeTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::gp::Effect<runir::kr::DlTag, FeatureTypeTag, ChangeTag>>>;

    Index<runir::kr::gp::EffectTag<FeatureTypeTag, ChangeTag>> index;
    Variant value;

    Data() = default;
    Data(Variant value_) : index(), value(std::move(value_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(value);
    }

    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

}  // namespace tyr

#endif
