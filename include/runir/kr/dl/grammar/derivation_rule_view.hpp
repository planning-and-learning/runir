#ifndef RUNIR_GRAMMAR_DERIVATION_RULE_VIEW_HPP_
#define RUNIR_GRAMMAR_DERIVATION_RULE_VIEW_HPP_

#include "runir/kr/dl/grammar/constructor_or_non_terminal_view.hpp"
#include "runir/kr/dl/grammar/derivation_rule_data.hpp"
#include "runir/kr/dl/grammar/non_terminal_view.hpp"

#include <tuple>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family,
         runir::kr::dl::CategoryTag Category,
         formalism::SymbolContextFor<runir::kr::dl::grammar::DerivationRule<Family, Category>> C>
class View<Index<runir::kr::dl::grammar::DerivationRule<Family, Category>>, C> :
    public ygg::IndexViewBase<runir::kr::dl::grammar::DerivationRule<Family, Category>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::grammar::DerivationRule<Family, Category>, C>::IndexViewBase;

    auto get_lhs() const noexcept { return make_view(this->get_data().lhs, this->get_context()); }
    auto get_rhs() const noexcept { return make_view(this->get_data().rhs, this->get_context()); }
};

}  // namespace ygg

#endif
