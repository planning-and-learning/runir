#ifndef RUNIR_GRAMMAR_GRAMMAR_VIEW_HPP_
#define RUNIR_GRAMMAR_GRAMMAR_VIEW_HPP_

#include "runir/kr/dl/grammar/grammar_data.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <tyr/formalism/planning/domain_view.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename C>
class View<Index<runir::kr::dl::grammar::GrammarTag<Family>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::dl::grammar::GrammarTag<Family>> m_handle;

public:
    View(Index<runir::kr::dl::grammar::GrammarTag<Family>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_domain() const noexcept { return make_view(get_data().domain, m_context->get_planning_repository()); }

    template<runir::kr::dl::CategoryTag Category>
    auto get_start() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return make_view(get_data().concept_start, *m_context);
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return make_view(get_data().role_start, *m_context);
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return make_view(get_data().boolean_start, *m_context);
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return make_view(get_data().numerical_start, *m_context);
    }

    template<runir::kr::dl::CategoryTag Category>
    auto get_derivation_rules() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return make_view(get_data().concept_derivation_rules, *m_context);
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return make_view(get_data().role_derivation_rules, *m_context);
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return make_view(get_data().boolean_derivation_rules, *m_context);
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return make_view(get_data().numerical_derivation_rules, *m_context);
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
