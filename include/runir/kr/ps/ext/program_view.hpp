#ifndef RUNIR_KR_PS_EXT_PROGRAM_VIEW_HPP_
#define RUNIR_KR_PS_EXT_PROGRAM_VIEW_HPP_

#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/program_data.hpp"
#include "runir/kr/ps/module_symbol_view.hpp"

#include <cstddef>
#include <optional>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace runir::kr::ps::ext
{
/// The syntactic complexity of a program (defined in syntactic_complexity.hpp), declared before its view, whose
/// functions() list it.
template<typename C>
std::size_t syntactic_complexity(const urd::index_view<Program, C>& view);
}  // namespace runir::kr::ps::ext

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::ext::Program, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto entry_mod() const noexcept { return self().template member<&urd::record_of<V>::entry_mod>(); }

    auto modules() const noexcept { return self().template member<&urd::record_of<V>::modules>(); }

    template<typename Symbol>
    auto find_module(Symbol symbol) const
    {
        using Module = decltype(entry_mod());
        for (auto module_ : modules())
            if (module_.symbol() == symbol)
                return std::optional<Module>(module_);
        return std::optional<Module>();
    }

    static constexpr auto functions() noexcept
    {
        return urd::Functions<urd::function<"syntactic_complexity", static_cast<std::size_t (*)(const V&)>(&runir::kr::ps::ext::syntactic_complexity)>,
                              urd::function<"find_module", &view_mixin::template find_module<runir::kr::ps::ext::ModuleSymbolView>, urd::arg<"symbol">>> {};
    }
};
}  // namespace urd

#endif
