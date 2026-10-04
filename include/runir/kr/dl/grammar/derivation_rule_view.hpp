#ifndef RUNIR_GRAMMAR_DERIVATION_RULE_VIEW_HPP_
#define RUNIR_GRAMMAR_DERIVATION_RULE_VIEW_HPP_

#include "runir/kr/dl/grammar/constructor_or_non_terminal_view.hpp"
#include "runir/kr/dl/grammar/derivation_rule_data.hpp"
#include "runir/kr/dl/grammar/non_terminal_view.hpp"

#include <tuple>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family,
         runir::kr::dl::CategoryTag Category,
         formalism::SymbolContextFor<runir::kr::dl::grammar::DerivationRule<Family, Category>> C>
class View<Index<runir::kr::dl::grammar::DerivationRule<Family, Category>>, C> :
    public formalism::detail::View<Index<runir::kr::dl::grammar::DerivationRule<Family, Category>>, C>
{
public:
    View(Index<runir::kr::dl::grammar::DerivationRule<Family, Category>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::dl::grammar::DerivationRule<Family, Category>>, C>(handle, context)
    {
    }

    auto get_lhs() const noexcept { return make_view(this->get_data().lhs, *this->m_context); }
    auto get_rhs() const noexcept { return make_view(this->get_data().rhs, *this->m_context); }
};

}  // namespace ygg

#endif
