#ifndef RUNIR_KR_PS_EXT_MODULE_VIEW_HPP_
#define RUNIR_KR_PS_EXT_MODULE_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/ps/ext/module_data.hpp"
#include "runir/kr/ps/module_view.hpp"

#include <concepts>
#include <string_view>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::ps::ext::Module, V> : runir::kr::ps::ModuleViewMixin<V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    template<runir::kr::dl::CategoryTag Category>
    auto get_arguments() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return urd::resolve(self().data().concept_arguments, get_repository(self().context()).get_dl_repository());
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return urd::resolve(self().data().role_arguments, get_repository(self().context()).get_dl_repository());
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return urd::resolve(self().data().boolean_arguments, get_repository(self().context()).get_dl_repository());
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return urd::resolve(self().data().numerical_arguments, get_repository(self().context()).get_dl_repository());
    }

    /// The rules of its :datalog section as written; empty without one.
    [[nodiscard]] std::string_view get_datalog() const noexcept { return self().data().datalog.view(); }
};
}  // namespace urd

#endif
