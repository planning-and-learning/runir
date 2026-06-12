#ifndef RUNIR_KR_PS_EXT_RULE_VARIANT_DATA_HPP_
#define RUNIR_KR_PS_EXT_RULE_VARIANT_DATA_HPP_

#include "runir/kr/ps/ext/rule_index.hpp"
#include "runir/kr/ps/ext/rule_variant_index.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <string>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <utility>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ext::RuleVariant>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::LoadTag>>,
                                             Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::SketchTag>>,
                                             Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::DoTag>>,
                                             Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>>>;

    Index<runir::kr::ps::ext::RuleVariant> index;
    ::cista::offset::string symbol;
    ::cista::offset::string description;
    Variant value;

    Data() = default;
    Data(Variant value_) : index(), value(std::move(value_)) {}
    Data(::cista::offset::string symbol_, ::cista::offset::string description_, Variant value_) :
        index(),
        symbol(std::move(symbol_)),
        description(std::move(description_)),
        value(std::move(value_))
    {
    }
    Data(const std::string& symbol_, const std::string& description_, Variant value_) :
        index(),
        symbol(symbol_),
        description(description_),
        value(std::move(value_))
    {
    }

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(symbol);
        ygg::clear(description);
        ygg::clear(value);
    }

    auto cista_members() const noexcept { return std::tie(index, symbol, description, value); }
    auto identifying_members() const noexcept { return std::tie(symbol, description, value); }
};

}  // namespace ygg

#endif
