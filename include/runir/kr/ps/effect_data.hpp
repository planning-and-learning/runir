#ifndef RUNIR_KR_PS_EFFECT_DATA_HPP_
#define RUNIR_KR_PS_EFFECT_DATA_HPP_

#include "runir/kr/ps/effect_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::FamilyTag Family>
struct Data<runir::kr::ps::EffectVariant<Family>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ConcreteEffectVariant<Family, runir::kr::DlTag>>>;

    Index<runir::kr::ps::EffectVariant<Family>> index;
    Variant variant;

    Data() = default;
    Data(Variant variant_) : index(), variant(std::move(variant_)) {}

    auto cista_members() noexcept { return std::tie(index, variant); }
    auto cista_members() const noexcept { return std::tie(index, variant); }
    auto identifying_members() const noexcept { return std::tie(variant); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
