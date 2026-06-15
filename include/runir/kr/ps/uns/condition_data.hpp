#ifndef RUNIR_KR_PS_UNS_CONDITION_DATA_HPP_
#define RUNIR_KR_PS_UNS_CONDITION_DATA_HPP_

#include "runir/kr/ps/condition_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ConditionVariant<runir::kr::UnsFamilyTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::UnsFamilyTag, runir::kr::DlTag>>>;

    Index<runir::kr::ps::ConditionVariant<runir::kr::UnsFamilyTag>> index;
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
