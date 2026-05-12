#ifndef RUNIR_KR_GP_CONDITION_DATA_HPP_
#define RUNIR_KR_GP_CONDITION_DATA_HPP_

#include "runir/kr/gp/condition_index.hpp"
#include "runir/kr/gp/dl/condition_data.hpp"
#include "runir/kr/gp/dl/declarations.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::gp::ConditionVariantTag>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::gp::ConditionTag<runir::kr::gp::dl::BooleanFeatureTag, runir::kr::gp::dl::PositiveTag>>,
                                             Index<runir::kr::gp::ConditionTag<runir::kr::gp::dl::BooleanFeatureTag, runir::kr::gp::dl::NegativeTag>>,
                                             Index<runir::kr::gp::ConditionTag<runir::kr::gp::dl::NumericalFeatureTag, runir::kr::gp::dl::EqualZeroTag>>,
                                             Index<runir::kr::gp::ConditionTag<runir::kr::gp::dl::NumericalFeatureTag, runir::kr::gp::dl::GreaterZeroTag>>>;

    Index<runir::kr::gp::ConditionVariantTag> index;
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

template<typename FeatureTypeTag, typename ObservationTag>
struct Data<runir::kr::gp::ConditionTag<FeatureTypeTag, ObservationTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::gp::Condition<runir::kr::DlTag, FeatureTypeTag, ObservationTag>>>;

    Index<runir::kr::gp::ConditionTag<FeatureTypeTag, ObservationTag>> index;
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
