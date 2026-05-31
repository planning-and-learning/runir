#ifndef RUNIR_KR_PS_BASE_CONDITION_DATA_HPP_
#define RUNIR_KR_PS_BASE_CONDITION_DATA_HPP_

#include "runir/kr/ps/condition_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>>;

    Index<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>> index;
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

}  // namespace ygg

#endif
