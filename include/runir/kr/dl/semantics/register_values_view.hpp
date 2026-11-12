#ifndef RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_HPP_
#define RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_HPP_

#include "runir/kr/dl/semantics/register_values_data.hpp"
#include "runir/kr/dl/semantics/register_values_view_concepts.hpp"

#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/object_view.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/core/config.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::dl::semantics::RegisterValues, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    /// The formalism repository: the context's, or the context itself for a register record held outside it.
    const auto& get_formalism_repository() const noexcept
    {
        if constexpr (requires { self().context().get_formalism_repository(); })
            return self().context().get_formalism_repository();
        else
            return self().context();
    }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto values() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return urd::resolve(self().data().concept_values, get_formalism_repository());
        else
            return urd::resolve(self().data().role_values, get_formalism_repository());
    }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto at(runir::kr::dl::RegisterIdentifier<Category> reg) const
    { return values<Category>().at(static_cast<size_t>(ygg::uint_t(reg.value))); }
};
}  // namespace urd

#endif
