#ifndef RUNIR_KR_PS_ICP_MODULE_SYMBOL_VIEW_HPP_
#define RUNIR_KR_PS_ICP_MODULE_SYMBOL_VIEW_HPP_

#include "runir/kr/ps/icp/module_symbol_data.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::icp::ModuleSymbol, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    const auto& get_name() const noexcept { return self().data().name; }
};
}  // namespace urd

#endif
