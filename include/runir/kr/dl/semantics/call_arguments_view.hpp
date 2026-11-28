#ifndef RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_VIEW_HPP_
#define RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_VIEW_HPP_

#include "runir/kr/dl/semantics/call_arguments_data.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"

#include <concepts>
#include <tuple>
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
    auto concept_arguments() const noexcept { return self().template member<&urd::record_of<V>::concept_arguments>(); }

    auto role_arguments() const noexcept { return self().template member<&urd::record_of<V>::role_arguments>(); }

    auto boolean_arguments() const noexcept { return self().template member<&urd::record_of<V>::boolean_arguments>(); }

    auto numerical_arguments() const noexcept { return self().template member<&urd::record_of<V>::numerical_arguments>(); }

    template<runir::kr::dl::CategoryTag Category>
    auto arguments() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return concept_arguments();
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return role_arguments();
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return boolean_arguments();
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return numerical_arguments();
        else
            static_assert(false, "unhandled argument category");
    }

    template<runir::kr::dl::CategoryTag Category>
    auto at(runir::kr::dl::ArgumentIdentifier<Category> arg) const
    { return arguments<Category>().at(static_cast<size_t>(ygg::uint_t(arg.value))); }

    static constexpr auto functions() noexcept
    {
        return urd::Functions<urd::function<"at", &view_mixin::template at<runir::kr::dl::ConceptTag>, urd::arg<"arg">>,
                              urd::function<"at", &view_mixin::template at<runir::kr::dl::RoleTag>, urd::arg<"arg">>,
                              urd::function<"at", &view_mixin::template at<runir::kr::dl::BooleanTag>, urd::arg<"arg">>,
                              urd::function<"at", &view_mixin::template at<runir::kr::dl::NumericalTag>, urd::arg<"arg">>> {};
    }
};
}  // namespace urd

#endif
