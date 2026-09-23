#ifndef RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_HPP_
#define RUNIR_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_HPP_

#include "runir/kr/dl/semantics/register_values_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/object_view.hpp>
#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/pair.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/dependent_false.hpp>

namespace ygg
{

template<typename C>
class View<Index<runir::kr::dl::semantics::RegisterValues>, C>
{
private:
    Index<runir::kr::dl::semantics::RegisterValues> m_handle;
    const C* m_context;

public:
    View(Index<runir::kr::dl::semantics::RegisterValues> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const noexcept { return get_denotation_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }
    auto get_index() const noexcept { return m_handle; }

    auto get_concept_values() const noexcept { return make_view(get_data().concept_values, get_context().get_formalism_repository()); }
    auto get_role_values() const noexcept { return make_view(get_data().role_values, get_context().get_formalism_repository()); }

    template<runir::kr::dl::CategoryTag Category>
    auto get() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return get_concept_values();
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return get_role_values();
        else
            static_assert(dependent_false<Category>::value, "unhandled register category");
    }

    template<runir::kr::dl::CategoryTag Category>
    auto at(runir::kr::dl::RegisterIdentifier<Category> reg) const
    {
        return get<Category>().at(static_cast<size_t>(ygg::uint_t(reg)));
    }

    auto identifying_members() const noexcept { return std::make_tuple(get_handle(), get_denotation_repository(*m_context).get_index()); }
};

}  // namespace ygg

#endif
