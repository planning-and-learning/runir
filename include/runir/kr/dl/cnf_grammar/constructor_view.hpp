#ifndef RUNIR_CNF_GRAMMAR_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_CONSTRUCTOR_VIEW_HPP_

#include "runir/kr/dl/cnf_grammar/boolean_view.hpp"
#include "runir/kr/dl/cnf_grammar/concept_view.hpp"
#include "runir/kr/dl/cnf_grammar/constructor_index.hpp"
#include "runir/kr/dl/cnf_grammar/numerical_view.hpp"
#include "runir/kr/dl/cnf_grammar/role_view.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family,
         runir::kr::dl::CategoryTag Category,
         formalism::SymbolContextFor<runir::kr::dl::cnf_grammar::Constructor<Family, Category>> C>
class View<Index<runir::kr::dl::cnf_grammar::Constructor<Family, Category>>, C> :
    public formalism::detail::View<Index<runir::kr::dl::cnf_grammar::Constructor<Family, Category>>, C>
{
public:
    View(Index<runir::kr::dl::cnf_grammar::Constructor<Family, Category>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::dl::cnf_grammar::Constructor<Family, Category>>, C>(handle, context)
    {
    }

    auto get_variant() const noexcept { return make_view(this->get_data().variant, *this->m_context); }
};

}

#endif
