#ifndef RUNIR_KR_PS_EXT_RULE_VARIANT_DATA_HPP_
#define RUNIR_KR_PS_EXT_RULE_VARIANT_DATA_HPP_

#include "runir/kr/ps/ext/rule_index.hpp"
#include "runir/kr/ps/ext/rule_variant_index.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <string>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::LoadTag<runir::kr::dl::ConceptTag>>>,
                                             Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::LoadTag<runir::kr::dl::RoleTag>>>,
                                             Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::SketchTag>>,
                                             Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::DoTag>>,
                                             Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>>>;

    Index<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>> index;
    ::cista::offset::string symbol;
    Variant variant;

    Data() = default;
    Data(Variant variant_) : index(), variant(std::move(variant_)) {}
    Data(::cista::offset::string symbol_, Variant variant_) : index(), symbol(std::move(symbol_)), variant(std::move(variant_)) {}
    Data(const std::string& symbol_, Variant variant_) : index(), symbol(symbol_), variant(std::move(variant_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(symbol);
        ygg::clear(variant);
    }

    auto cista_members() const noexcept { return std::tie(index, symbol, variant); }
    auto identifying_members() const noexcept { return std::tie(symbol, variant); }
};

}  // namespace ygg

#endif
