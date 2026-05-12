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
struct Data<runir::kr::gp::ConditionVariant>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::gp::ConcreteConditionVariant<runir::kr::DlTag>>>;

    Index<runir::kr::gp::ConditionVariant> index;
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
struct Data<runir::kr::gp::ConcreteConditionVariant<runir::kr::DlTag>>
{
    using Variant = ::cista::offset::variant<
        Index<runir::kr::gp::ConcreteCondition<runir::kr::DlTag, runir::kr::gp::dl::BooleanFeature, runir::kr::gp::dl::Positive>>,
        Index<runir::kr::gp::ConcreteCondition<runir::kr::DlTag, runir::kr::gp::dl::BooleanFeature, runir::kr::gp::dl::Negative>>,
        Index<runir::kr::gp::ConcreteCondition<runir::kr::DlTag, runir::kr::gp::dl::NumericalFeature, runir::kr::gp::dl::EqualZero>>,
        Index<runir::kr::gp::ConcreteCondition<runir::kr::DlTag, runir::kr::gp::dl::NumericalFeature, runir::kr::gp::dl::GreaterZero>>>;

    Index<runir::kr::gp::ConcreteConditionVariant<runir::kr::DlTag>> index;
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
