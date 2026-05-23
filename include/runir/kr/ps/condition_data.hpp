#ifndef RUNIR_KR_PS_CONDITION_DATA_HPP_
#define RUNIR_KR_PS_CONDITION_DATA_HPP_

#include "runir/kr/ps/condition_index.hpp"
#include "runir/kr/ps/base/dl/condition_data.hpp"
#include "runir/kr/ps/base/dl/declarations.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::ps::ConditionVariant>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::DlTag>>>;

    Index<runir::kr::ps::ConditionVariant> index;
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
struct Data<runir::kr::ps::ConcreteConditionVariant<runir::kr::DlTag>>
{
    using Variant = ::cista::offset::variant<
        Index<runir::kr::ps::ConcreteCondition<runir::kr::DlTag, runir::kr::ps::base::dl::BooleanFeature, runir::kr::ps::base::dl::Positive>>,
        Index<runir::kr::ps::ConcreteCondition<runir::kr::DlTag, runir::kr::ps::base::dl::BooleanFeature, runir::kr::ps::base::dl::Negative>>,
        Index<runir::kr::ps::ConcreteCondition<runir::kr::DlTag, runir::kr::ps::base::dl::NumericalFeature, runir::kr::ps::base::dl::EqualZero>>,
        Index<runir::kr::ps::ConcreteCondition<runir::kr::DlTag, runir::kr::ps::base::dl::NumericalFeature, runir::kr::ps::base::dl::GreaterZero>>>;

    Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::DlTag>> index;
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
