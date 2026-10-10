#ifndef RUNIR_CNF_GRAMMAR_SUBSTITUTION_RULE_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_SUBSTITUTION_RULE_VIEW_HPP_

#include "runir/kr/dl/cnf_grammar/non_terminal_view.hpp"
#include "runir/kr/dl/cnf_grammar/substitution_rule_data.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family,
         runir::kr::dl::CategoryTag Category,
         formalism::SymbolContextFor<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, Category>> C>
class View<Index<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, Category>>, C> :
    public ygg::IndexViewBase<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, Category>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, Category>, C>::IndexViewBase;

    auto get_lhs() const noexcept { return make_view(this->get_data().lhs, this->get_context()); }
    auto get_rhs() const noexcept { return make_view(this->get_data().rhs, this->get_context()); }
};

}  // namespace ygg

#endif
