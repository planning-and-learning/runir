#ifndef RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_VIEW_HPP_
#define RUNIR_KR_DL_SEMANTICS_CALL_ARGUMENTS_VIEW_HPP_

#include "runir/kr/dl/semantics/call_arguments_data.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/dependent_false.hpp>

namespace ygg
{

template<typename C>
class View<Index<runir::kr::dl::semantics::CallArguments>, C>
{
private:
    Index<runir::kr::dl::semantics::CallArguments> m_handle;
    const C* m_context;

public:
    View(Index<runir::kr::dl::semantics::CallArguments> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const noexcept { return get_denotation_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }
    auto get_index() const noexcept { return m_handle; }

    template<runir::kr::dl::CategoryTag Category>
    auto get() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return make_view(get_data().concept_arguments, get_context());
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return make_view(get_data().role_arguments, get_context());
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return make_view(get_data().boolean_arguments, get_context());
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return make_view(get_data().numerical_arguments, get_context());
        else
            static_assert(dependent_false<Category>::value, "unhandled argument category");
    }

    template<runir::kr::dl::CategoryTag Category>
    auto at(runir::kr::dl::ArgumentIdentifier<Category> arg) const
    {
        return get<Category>().at(static_cast<size_t>(ygg::uint_t(arg)));
    }

    auto identifying_members() const noexcept { return std::make_tuple(get_handle(), get_denotation_repository(*m_context).get_index()); }
};

}  // namespace ygg

#endif
