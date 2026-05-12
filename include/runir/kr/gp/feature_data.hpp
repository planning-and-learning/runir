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
struct Data<runir::kr::gp::Feature<runir::kr::gp::dl::BooleanFeature>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::gp::ConcreteFeature<runir::kr::DlTag, runir::kr::gp::dl::BooleanFeature>>>;

    Index<runir::kr::gp::Feature<runir::kr::gp::dl::BooleanFeature>> index;
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
struct Data<runir::kr::gp::Feature<runir::kr::gp::dl::NumericalFeature>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::gp::ConcreteFeature<runir::kr::DlTag, runir::kr::gp::dl::NumericalFeature>>>;

    Index<runir::kr::gp::Feature<runir::kr::gp::dl::NumericalFeature>> index;
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
