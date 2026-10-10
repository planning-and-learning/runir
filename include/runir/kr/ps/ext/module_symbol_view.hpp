#ifndef RUNIR_KR_PS_EXT_MODULE_SYMBOL_VIEW_HPP_
#define RUNIR_KR_PS_EXT_MODULE_SYMBOL_VIEW_HPP_

#include "runir/kr/ps/ext/module_symbol_data.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<formalism::SymbolContextFor<runir::kr::ps::ext::ModuleSymbol> C>
class View<Index<runir::kr::ps::ext::ModuleSymbol>, C> : public ygg::IndexViewBase<runir::kr::ps::ext::ModuleSymbol, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::ext::ModuleSymbol, C>::IndexViewBase;

    const auto& get_name() const noexcept { return this->get_data().name; }
};

}  // namespace ygg

#endif
