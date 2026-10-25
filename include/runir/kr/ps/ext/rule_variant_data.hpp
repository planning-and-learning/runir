#ifndef RUNIR_KR_PS_EXT_RULE_VARIANT_DATA_HPP_
#define RUNIR_KR_PS_EXT_RULE_VARIANT_DATA_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>
{
    using Variant = ::ygg::IndexVariant<runir::kr::ps::ext::ConcreteRuleTypes>;

    Index<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>> index;
    ::cista::offset::string symbol;
    Variant variant;

    Data() = default;
    Data(::cista::offset::string symbol_, Variant variant_) : index(), symbol(std::move(symbol_)), variant(std::move(variant_)) {}
    template<typename C>
    using ViewVariant = ::ygg::ViewVariant<Variant, C>;
    template<typename C>
    Data(::cista::offset::string symbol_, const ViewVariant<C>& variant_) :
        index(),
        symbol(std::move(symbol_)),
        variant(std::visit([](const auto& view) -> Variant { return Variant(view.get_index()); }, variant_))
    {
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::symbol, &Data::variant> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::symbol, &Data::variant> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

}  // namespace ygg

#endif
