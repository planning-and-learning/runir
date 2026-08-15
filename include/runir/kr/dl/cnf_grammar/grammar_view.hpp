#ifndef RUNIR_CNF_GRAMMAR_GRAMMAR_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_GRAMMAR_VIEW_HPP_

#include "runir/kr/dl/cnf_grammar/grammar_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/planning/domain_view.hpp>
#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, typename C>
class View<Index<runir::kr::dl::cnf_grammar::Grammar<Family>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::dl::cnf_grammar::Grammar<Family>> m_handle;

public:
    View(Index<runir::kr::dl::cnf_grammar::Grammar<Family>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_domain() const noexcept { return make_view(get_data().domain, m_context->get_planning_repository()); }

    template<runir::kr::dl::CategoryTag Category>
    auto get_start() const noexcept
    {
        return make_view(get_data().template get_start<Category>(), *m_context);
    }

    template<runir::kr::dl::CategoryTag Category>
    auto get_derivation_rules() const noexcept
    {
        return make_view(get_data().template get_derivation_rules<Category>(), *m_context);
    }

    template<runir::kr::dl::CategoryTag Category>
    auto get_substitution_rules() const noexcept
    {
        return make_view(get_data().template get_substitution_rules<Category>(), *m_context);
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
