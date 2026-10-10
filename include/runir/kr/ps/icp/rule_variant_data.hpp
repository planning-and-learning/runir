#ifndef RUNIR_KR_PS_ICP_RULE_VARIANT_DATA_HPP_
#define RUNIR_KR_PS_ICP_RULE_VARIANT_DATA_HPP_

#include "runir/kr/ps/icp/rule_index.hpp"
#include "runir/kr/ps/icp/rule_variant_index.hpp"

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
struct Data<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::ps::icp::Rule<runir::kr::ps::icp::LoadTag<runir::kr::dl::ConceptTag>>>,
                                             Index<runir::kr::ps::icp::Rule<runir::kr::ps::icp::LoadTag<runir::kr::dl::RoleTag>>>,
                                             Index<runir::kr::ps::icp::Rule<runir::kr::ps::icp::CruleTag>>>;

    Index<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>> index;
    ::cista::offset::string symbol;
    Variant variant;

    Data() = default;
    Data(::cista::offset::string symbol_, Variant variant_) : index(), symbol(std::move(symbol_)), variant(std::move(variant_)) {}

    auto cista_members() noexcept { return std::tie(index, symbol, variant); }
    auto cista_members() const noexcept { return std::tie(index, symbol, variant); }
    auto identifying_members() const noexcept { return std::tie(symbol, variant); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
