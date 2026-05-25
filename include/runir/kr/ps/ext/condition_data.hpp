#ifndef RUNIR_KR_PS_EXT_CONDITION_DATA_HPP_
#define RUNIR_KR_PS_EXT_CONDITION_DATA_HPP_

#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/ext/condition_index.hpp"
#include "runir/kr/ps/ext/dl/condition_data.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <utility>

namespace tyr
{

template<>
struct Data<runir::kr::ps::ext::ConditionVariant>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ext::ConcreteConditionVariant<runir::kr::DlTag>>>;

    Index<runir::kr::ps::ext::ConditionVariant> index;
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
struct Data<runir::kr::ps::ext::ConcreteConditionVariant<runir::kr::DlTag>>
{
    using Variant = ::cista::offset::variant<
        Index<runir::kr::ps::ext::ConcreteCondition<runir::kr::DlTag, runir::kr::dl::ConceptTag, runir::kr::ps::dl::EqualZero>>,
        Index<runir::kr::ps::ext::ConcreteCondition<runir::kr::DlTag, runir::kr::dl::ConceptTag, runir::kr::ps::dl::GreaterZero>>,
        Index<runir::kr::ps::ext::ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>>,
        Index<runir::kr::ps::ext::ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>>,
        Index<runir::kr::ps::ext::ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>>,
        Index<runir::kr::ps::ext::ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>>;

    Index<runir::kr::ps::ext::ConcreteConditionVariant<runir::kr::DlTag>> index;
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
