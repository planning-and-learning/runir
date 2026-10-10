#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_VIEW_HPP_

#include "runir/kr/dl/grammar/boolean_view.hpp"
#include "runir/kr/dl/grammar/concept_view.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"
#include "runir/kr/dl/grammar/numerical_view.hpp"
#include "runir/kr/dl/grammar/role_view.hpp"

#include <tuple>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family,
         runir::kr::dl::CategoryTag Category,
         formalism::SymbolContextFor<runir::kr::dl::grammar::Constructor<Family, Category>> C>
class View<Index<runir::kr::dl::grammar::Constructor<Family, Category>>, C> :
    public ygg::IndexViewBase<runir::kr::dl::grammar::Constructor<Family, Category>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::dl::grammar::Constructor<Family, Category>, C>::IndexViewBase;

    auto get_variant() const noexcept { return make_view(this->get_data().variant, this->get_context()); }
};

}

#endif
