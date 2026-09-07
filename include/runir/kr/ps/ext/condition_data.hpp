#ifndef RUNIR_KR_PS_EXT_CONDITION_DATA_HPP_
#define RUNIR_KR_PS_EXT_CONDITION_DATA_HPP_

#include "runir/kr/ps/condition_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>>;

    Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> index;
    Variant variant;

    Data() = default;
    Data(Variant variant_) : index(), variant(std::move(variant_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(variant);
    }

    auto cista_members() const noexcept { return std::tie(index, variant); }
    auto identifying_members() const noexcept { return std::tie(variant); }
};

}  // namespace ygg

#endif
