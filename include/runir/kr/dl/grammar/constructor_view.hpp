#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_VIEW_HPP_

#include "runir/kr/dl/grammar/boolean_view.hpp"
#include "runir/kr/dl/grammar/concept_view.hpp"
#include "runir/kr/dl/grammar/constructor_index.hpp"
#include "runir/kr/dl/grammar/numerical_view.hpp"
#include "runir/kr/dl/grammar/role_view.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family,
         runir::kr::dl::CategoryTag Category,
         formalism::SymbolContextFor<runir::kr::dl::grammar::Constructor<Family, Category>> C>
class View<Index<runir::kr::dl::grammar::Constructor<Family, Category>>, C> :
    public formalism::detail::View<Index<runir::kr::dl::grammar::Constructor<Family, Category>>, C>
{
public:
    View(Index<runir::kr::dl::grammar::Constructor<Family, Category>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::dl::grammar::Constructor<Family, Category>>, C>(handle, context)
    {
    }

    auto get_variant() const noexcept { return make_view(this->get_data().variant, *this->m_context); }
};

}

#endif
