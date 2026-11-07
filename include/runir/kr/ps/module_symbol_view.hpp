#ifndef RUNIR_KR_PS_MODULE_SYMBOL_VIEW_HPP_
#define RUNIR_KR_PS_MODULE_SYMBOL_VIEW_HPP_

#include "runir/kr/ps/module_symbol_data.hpp"

#include <tuple>
#include <yggdrasil/formalism/declarations.hpp>

template<runir::kr::ps::ProgramFamilyTag Family, typename V>
struct urd::view_mixin<runir::kr::ps::ModuleSymbol<Family>, V>
{
    const auto& get_name() const noexcept { return static_cast<const V&>(*this).data().name; }
};

#endif
