#ifndef RUNIR_CNF_GRAMMAR_GRAMMAR_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_GRAMMAR_VIEW_HPP_

#include "runir/kr/dl/cnf_grammar/grammar_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/planning/domain_view.hpp>
#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, formalism::SymbolContextFor<runir::kr::dl::cnf_grammar::Grammar<Family>> C>
class View<Index<runir::kr::dl::cnf_grammar::Grammar<Family>>, C> : public ygg::IndexViewBase<runir::kr::dl::cnf_grammar::Grammar<Family>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::cnf_grammar::Grammar<Family>, C>::IndexViewBase;

    auto get_domain() const noexcept { return make_view(this->get_data().domain, this->get_context().get_planning_repository()); }

    template<runir::kr::dl::CategoryTag Category>
    auto get_start() const noexcept
    {
        return make_view(this->get_data().template get_start<Category>(), this->get_context());
    }

    template<runir::kr::dl::CategoryTag Category>
    auto get_derivation_rules() const noexcept
    {
        return make_view(this->get_data().template get_derivation_rules<Category>(), this->get_context());
    }

    template<runir::kr::dl::CategoryTag Category>
    auto get_substitution_rules() const noexcept
    {
        return make_view(this->get_data().template get_substitution_rules<Category>(), this->get_context());
    }
};

}  // namespace ygg

#endif
