#ifndef RUNIR_KR_PS_EXT_PROGRAM_VIEW_HPP_
#define RUNIR_KR_PS_EXT_PROGRAM_VIEW_HPP_

#include "runir/kr/ps/ext/module_symbol_view.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/program_data.hpp"

#include <optional>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::ext::Program, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_entry_module() const noexcept { return self().template get<&urd::record_of<V>::entry_module>(); }
    auto get_modules() const noexcept { return self().template get<&urd::record_of<V>::modules>(); }
    template<typename Symbol>
    auto find_module(Symbol symbol) const
    {
        using Module = decltype(get_entry_module());
        for (auto module_ : get_modules())
            if (module_.get_symbol() == symbol)
                return std::optional<Module>(module_);
        return std::optional<Module>();
    }
};
}  // namespace urd

#endif
