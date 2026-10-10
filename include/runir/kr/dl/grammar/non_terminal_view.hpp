#ifndef RUNIR_GRAMMAR_NON_TERMINAL_VIEW_HPP_
#define RUNIR_GRAMMAR_NON_TERMINAL_VIEW_HPP_

#include "runir/kr/dl/grammar/non_terminal_data.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family,
         runir::kr::dl::CategoryTag Category,
         formalism::SymbolContextFor<runir::kr::dl::grammar::NonTerminal<Family, Category>> C>
class View<Index<runir::kr::dl::grammar::NonTerminal<Family, Category>>, C> :
    public ygg::IndexViewBase<runir::kr::dl::grammar::NonTerminal<Family, Category>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::grammar::NonTerminal<Family, Category>, C>::IndexViewBase;

    const auto& get_name() const noexcept { return this->get_data().name; }
};

}  // namespace ygg

#endif
