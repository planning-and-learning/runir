#ifndef RUNIR_GRAMMAR_NON_TERMINAL_VIEW_HPP_
#define RUNIR_GRAMMAR_NON_TERMINAL_VIEW_HPP_

#include "runir/kr/dl/grammar/non_terminal_data.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family,
         runir::kr::dl::CategoryTag Category,
         formalism::SymbolContextFor<runir::kr::dl::grammar::NonTerminal<Family, Category>> C>
class View<Index<runir::kr::dl::grammar::NonTerminal<Family, Category>>, C> :
    public formalism::detail::View<Index<runir::kr::dl::grammar::NonTerminal<Family, Category>>, C>
{
public:
    View(Index<runir::kr::dl::grammar::NonTerminal<Family, Category>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::dl::grammar::NonTerminal<Family, Category>>, C>(handle, context)
    {
    }

    const auto& get_name() const noexcept { return this->get_data().name; }
};

}  // namespace ygg

#endif
