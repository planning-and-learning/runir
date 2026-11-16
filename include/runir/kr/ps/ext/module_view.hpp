#ifndef RUNIR_KR_PS_EXT_MODULE_VIEW_HPP_
#define RUNIR_KR_PS_EXT_MODULE_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/ps/ext/module_data.hpp"
#include "runir/kr/ps/module_view.hpp"

#include <concepts>
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
            return self().template member<&urd::record_of<V>::concept_arguments>();
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return self().template member<&urd::record_of<V>::role_arguments>();
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return self().template member<&urd::record_of<V>::boolean_arguments>();
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return self().template member<&urd::record_of<V>::numerical_arguments>();
    }
};
}  // namespace urd

#endif
