#ifndef RUNIR_KR_PS_EXT_PROGRAM_VIEW_HPP_
#define RUNIR_KR_PS_EXT_PROGRAM_VIEW_HPP_

#include "runir/kr/ps/ext/module_symbol_view.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/program_data.hpp"

#include <optional>
#include <tuple>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<formalism::SymbolContextFor<runir::kr::ps::ext::Program> C>
class View<Index<runir::kr::ps::ext::Program>, C> : public ygg::IndexViewBase<runir::kr::ps::ext::Program, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::ext::Program, C>::IndexViewBase;

    auto get_entry_module() const noexcept { return View<Index<runir::kr::ps::ext::Module>, C>(this->get_data().entry_module, this->get_context()); }
    auto get_modules() const noexcept { return make_view(this->get_data().modules, this->get_context()); }
    std::optional<View<Index<runir::kr::ps::ext::Module>, C>> find_module(View<Index<runir::kr::ps::ext::ModuleSymbol>, C> symbol) const
    {
        for (auto module_ : get_modules())
            if (module_.get_symbol() == symbol)
                return module_;
        return std::nullopt;
    }
};

}  // namespace ygg

#endif
