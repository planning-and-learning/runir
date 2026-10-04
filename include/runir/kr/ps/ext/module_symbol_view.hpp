#ifndef RUNIR_KR_PS_EXT_MODULE_SYMBOL_VIEW_HPP_
#define RUNIR_KR_PS_EXT_MODULE_SYMBOL_VIEW_HPP_

#include "runir/kr/ps/ext/module_symbol_data.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<formalism::SymbolContextFor<runir::kr::ps::ext::ModuleSymbol> C>
class View<Index<runir::kr::ps::ext::ModuleSymbol>, C> : public formalism::detail::View<Index<runir::kr::ps::ext::ModuleSymbol>, C>
{
public:
    View(Index<runir::kr::ps::ext::ModuleSymbol> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::ps::ext::ModuleSymbol>, C>(handle, context)
    {
    }

    const auto& get_name() const noexcept { return this->get_data().name; }
};

}  // namespace ygg

#endif
