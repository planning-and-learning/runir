#ifndef RUNIR_KR_GP_FEATURE_DATA_HPP_
#define RUNIR_KR_GP_FEATURE_DATA_HPP_

#include "runir/kr/gp/dl/declarations.hpp"
#include "runir/kr/gp/dl/feature_index.hpp"
#include "runir/kr/gp/feature_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::gp::FeatureTag<runir::kr::gp::dl::BooleanFeatureTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::gp::Feature<runir::kr::DlTag, runir::kr::gp::dl::BooleanFeatureTag>>>;

    Index<runir::kr::gp::FeatureTag<runir::kr::gp::dl::BooleanFeatureTag>> index;
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

template<>
struct Data<runir::kr::gp::FeatureTag<runir::kr::gp::dl::NumericalFeatureTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::gp::Feature<runir::kr::DlTag, runir::kr::gp::dl::NumericalFeatureTag>>>;

    Index<runir::kr::gp::FeatureTag<runir::kr::gp::dl::NumericalFeatureTag>> index;
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
