#ifndef RUNIR_KR_PS_EXT_MODULE_VIEW_HPP_
#define RUNIR_KR_PS_EXT_MODULE_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/module_data.hpp"
#include "runir/kr/ps/ext/module_symbol_view.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<formalism::SymbolContextFor<runir::kr::ps::ext::Module> C>
class View<Index<runir::kr::ps::ext::Module>, C> : public formalism::detail::View<Index<runir::kr::ps::ext::Module>, C>
{
public:
    View(Index<runir::kr::ps::ext::Module> handle, const C& context) noexcept : formalism::detail::View<Index<runir::kr::ps::ext::Module>, C>(handle, context)
    {
    }

    auto get_symbol() const noexcept { return make_view(this->get_data().symbol, *this->m_context); }
    const auto& get_name() const noexcept { return get_symbol().get_name(); }

    template<runir::kr::dl::CategoryTag Category>
    auto get_arguments() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return make_view(this->get_data().concept_arguments, get_repository(*this->m_context).get_dl_repository());
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return make_view(this->get_data().role_arguments, get_repository(*this->m_context).get_dl_repository());
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return make_view(this->get_data().boolean_arguments, get_repository(*this->m_context).get_dl_repository());
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return make_view(this->get_data().numerical_arguments, get_repository(*this->m_context).get_dl_repository());
    }

    template<runir::kr::dl::CategoryTag Category>
    auto get_registers() const noexcept
        requires(std::same_as<Category, runir::kr::dl::ConceptTag> || std::same_as<Category, runir::kr::dl::RoleTag>)
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return make_view(this->get_data().concept_registers, get_repository(*this->m_context).get_dl_repository());
        else
            return make_view(this->get_data().role_registers, get_repository(*this->m_context).get_dl_repository());
    }

    template<typename FeatureTag>
    auto get_features() const noexcept
    {
        if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
            return make_view(this->get_data().concept_features, *this->m_context);
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
            return make_view(this->get_data().role_features, *this->m_context);
        else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
            return make_view(this->get_data().boolean_features, *this->m_context);
        else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
            return make_view(this->get_data().numerical_features, *this->m_context);
        else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::QueryFeature>)
            return make_view(this->get_data().query_features, *this->m_context);
        else
        {
            static_assert(ygg::dependent_false<FeatureTag>::value, "unhandled feature tag in Module::get_features");
        }
    }

    auto get_query_features() const noexcept { return get_features<runir::kr::ps::dl::QueryFeature>(); }

    auto get_entry_memory_state() const noexcept { return make_view(this->get_data().entry_memory_state, *this->m_context); }
    auto get_memory_states() const noexcept { return make_view(this->get_data().memory_states, *this->m_context); }
    auto get_memory_transitions() const noexcept { return make_view(this->get_data().memory_transitions, *this->m_context); }
};

}  // namespace ygg

#endif
