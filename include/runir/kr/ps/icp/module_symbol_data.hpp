#ifndef RUNIR_KR_PS_ICP_MODULE_SYMBOL_DATA_HPP_
#define RUNIR_KR_PS_ICP_MODULE_SYMBOL_DATA_HPP_

#include "runir/kr/ps/icp/declarations.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::icp::ModuleSymbol>
{
    Index<runir::kr::ps::icp::ModuleSymbol> index;
    ::cista::offset::string name;

    Data() = default;
    Data(::cista::offset::string name_) : index(), name(std::move(name_)) {}

    auto cista_members() noexcept { return std::tie(index, name); }
    auto cista_members() const noexcept { return std::tie(index, name); }
    auto identifying_members() const noexcept { return std::tie(name); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
