#ifndef RUNIR_KR_PS_EXT_RULE_VARIANT_DATA_HPP_
#define RUNIR_KR_PS_EXT_RULE_VARIANT_DATA_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>

namespace urd
{

template<>
struct data<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>
{
    using Variant = urd::index_variant<runir::kr::ps::ext::ConcreteRuleTypes>;

    urd::index<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>> index;
    ::cista::offset::string symbol;
    Variant variant;

    data() = default;
    data(::cista::offset::string symbol_, Variant variant_) : index(), symbol(std::move(symbol_)), variant(std::move(variant_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::symbol, &data::variant> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::symbol, &data::variant> {}; }
};

}  // namespace urd

#endif
