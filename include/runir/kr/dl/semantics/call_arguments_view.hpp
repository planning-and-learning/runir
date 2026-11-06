#ifndef RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_VIEW_HPP_
#define RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_VIEW_HPP_

#include "runir/kr/dl/semantics/call_arguments_data.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"

#include <concepts>
#include <tuple>
#include <urd/repr/core/dependent_false.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/core/config.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::dl::semantics::CallArguments, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    template<runir::kr::dl::CategoryTag Category>
    auto arguments() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return urd::resolve(self().data().concept_arguments, self().context());
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return urd::resolve(self().data().role_arguments, self().context());
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return urd::resolve(self().data().boolean_arguments, self().context());
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return urd::resolve(self().data().numerical_arguments, self().context());
        else
            static_assert(urd::repr::dependent_false<Category>::value, "unhandled argument category");
    }

    template<runir::kr::dl::CategoryTag Category>
    auto at(runir::kr::dl::ArgumentIdentifier<Category> arg) const
    { return arguments<Category>().at(static_cast<size_t>(ygg::uint_t(arg.value))); }
};
}  // namespace urd

#endif
