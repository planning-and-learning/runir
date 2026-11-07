#ifndef RUNIR_KR_PS_ICP_MODULE_SYMBOL_DATA_HPP_
#define RUNIR_KR_PS_ICP_MODULE_SYMBOL_DATA_HPP_

#include "runir/kr/ps/icp/declarations.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <utility>
#include <urd/elements/flat.hpp>

namespace urd
{

template<>
struct data<runir::kr::ps::icp::ModuleSymbol>
{
    urd::index<runir::kr::ps::icp::ModuleSymbol> index;
    urd::flat_string name;

    data() = default;
    data(urd::flat_string name_) : index(), name(std::move(name_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::name> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::name> {}; }
};

}  // namespace urd

#endif
