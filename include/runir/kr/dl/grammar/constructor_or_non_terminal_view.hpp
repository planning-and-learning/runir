#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_OR_NON_TERMINAL_VIEW_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_OR_NON_TERMINAL_VIEW_HPP_

#include "runir/kr/dl/grammar/constructor_or_non_terminal_data.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family,
         runir::kr::dl::CategoryTag Category,
         formalism::SymbolContextFor<runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, Category>> C>
class View<Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, Category>>, C> :
    public ygg::IndexViewBase<runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, Category>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, Category>, C>::IndexViewBase;

    auto get_variant() const noexcept { return make_view(this->get_data().variant, this->get_context()); }
};

}  // namespace ygg

#endif
