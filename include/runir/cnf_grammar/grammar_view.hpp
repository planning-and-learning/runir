#ifndef RUNIR_CNF_GRAMMAR_GRAMMAR_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_GRAMMAR_VIEW_HPP_

#include "runir/cnf_grammar/grammar_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/common/optional.hpp>
#include <tyr/common/types.hpp>
#include <tyr/common/vector.hpp>
#include <tyr/formalism/planning/domain_view.hpp>

namespace tyr
{

template<typename C>
class View<Index<runir::cnf_grammar::GrammarTag>, C>
{
private:
    const C* m_context;
    Index<runir::cnf_grammar::GrammarTag> m_handle;

public:
    View(Index<runir::cnf_grammar::GrammarTag> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_domain() const noexcept { return make_view(get_data().domain, *m_context); }

    template<runir::CategoryTag Category>
    auto get_start() const noexcept
    {
        if constexpr (std::same_as<Category, runir::ConceptTag>)
            return make_view(get_data().concept_start, *m_context);
        else if constexpr (std::same_as<Category, runir::RoleTag>)
            return make_view(get_data().role_start, *m_context);
        else if constexpr (std::same_as<Category, runir::BooleanTag>)
            return make_view(get_data().boolean_start, *m_context);
        else if constexpr (std::same_as<Category, runir::NumericalTag>)
            return make_view(get_data().numerical_start, *m_context);
    }

    template<runir::CategoryTag Category>
    auto get_derivation_rules() const noexcept
    {
        if constexpr (std::same_as<Category, runir::ConceptTag>)
            return make_view(get_data().concept_derivation_rules, *m_context);
        else if constexpr (std::same_as<Category, runir::RoleTag>)
            return make_view(get_data().role_derivation_rules, *m_context);
        else if constexpr (std::same_as<Category, runir::BooleanTag>)
            return make_view(get_data().boolean_derivation_rules, *m_context);
        else if constexpr (std::same_as<Category, runir::NumericalTag>)
            return make_view(get_data().numerical_derivation_rules, *m_context);
    }

    template<runir::CategoryTag Category>
    auto get_substitution_rules() const noexcept
    {
        if constexpr (std::same_as<Category, runir::ConceptTag>)
            return make_view(get_data().concept_substitution_rules, *m_context);
        else if constexpr (std::same_as<Category, runir::RoleTag>)
            return make_view(get_data().role_substitution_rules, *m_context);
        else if constexpr (std::same_as<Category, runir::BooleanTag>)
            return make_view(get_data().boolean_substitution_rules, *m_context);
        else if constexpr (std::same_as<Category, runir::NumericalTag>)
            return make_view(get_data().numerical_substitution_rules, *m_context);
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
